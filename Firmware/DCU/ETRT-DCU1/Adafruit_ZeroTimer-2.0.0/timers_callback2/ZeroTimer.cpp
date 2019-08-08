#include "ZeroTimer.h"
#include "wiring_private.h"

// mostly from asfdoc_sam0_tc_basic_use_case.html

#ifndef TC_INST_MAX_ID
#define TC_INST_MAX_ID 5
#endif

#ifndef TC_INSTANCE_OFFSET
#define TC_INSTANCE_OFFSET 3
#endif

extern "C" {

#define TC_MAX_CALLBACKS (TC_CALLBACK_BITS*3)
#define TC_CALLBACK_BITS 6
static void(* __cb[TC_MAX_CALLBACKS]) (void);
/** Bit mask for callbacks registered */
static uint32_t _register_callback_mask = 0;
/** Bit mask for callbacks enabled */
static uint32_t _enable_callback_mask = 0;

};

static inline bool tc_is_syncing(Tc *const hw)
{
#if defined(__SAMD51__)
  return hw->COUNT8.SYNCBUSY.reg > 0;
#else
  return (hw->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY);
#endif
}

Adafruit_ZeroTimer::Adafruit_ZeroTimer(uint8_t timernum)
{
  _timernum = timernum;
}

bool Adafruit_ZeroTimer::tc_init()
{
  /* Temporary variable to hold all updates to the CTRLA
	 * register before they are written to it */
  uint16_t ctrla_tmp = 0;
  /* Temporary variable to hold all updates to the CTRLBSET
	 * register before they are written to it */
  uint8_t ctrlbset_tmp = 0;
  /* Temporary variable to hold all updates to the CTRLC
	 * register before they are written to it */
  uint8_t ctrlc_tmp = 0;
  /* Temporary variable to hold TC instance number */
  uint8_t instance = _timernum - TC_INSTANCE_OFFSET;

#if defined(__SAMD51__)
/* Array of GLCK ID for different TC instances */
  uint32_t inst_gclk_id[] = { TC3_GCLK_ID, TC4_GCLK_ID, TC5_GCLK_ID };
 #else
  /* Array of GLCK ID for different TC instances */
  uint32_t inst_gclk_id[] = {GCLK_CLKCTRL_ID(GCM_TCC2_TC3), GCLK_CLKCTRL_ID(GCM_TC4_TC5), GCLK_CLKCTRL_ID(GCM_TC4_TC5)};
  /* Array of PM APBC mask bit position for different TC instances */
  uint32_t inst_pm_apbmask[] = {PM_APBCMASK_TC3, PM_APBCMASK_TC4, PM_APBCMASK_TC5};
#endif

  /* Initialize parameters */
  uint32_t cbmask = 0;
  for (uint8_t i = 0; i < TC_CALLBACK_BITS; i++)
  {
    __cb[(instance*TC_CALLBACK_BITS) + i] = NULL;
    cbmask |= (1UL << ((instance*TC_CALLBACK_BITS) + i));
  }
  _register_callback_mask &= ~cbmask;
  _enable_callback_mask &= ~cbmask;

  /* Check if odd numbered TC modules are being configured in 32-bit
	 * counter size. Only even numbered counters are allowed to be
	 * configured in 32-bit counter size.
	 */
  if ((_counter_size == TC_COUNTER_SIZE_32BIT) &&
      ((instance + TC_INSTANCE_OFFSET) & 0x01))
  {
    return false;
  }

  if (_hw->COUNT8.CTRLA.reg & TC_CTRLA_SWRST)
  {
    /* We are in the middle of a reset. Abort. */
    return false;
  }

  if (_hw->COUNT8.STATUS.reg & TC_STATUS_SLAVE)
  {
    /* Module is used as a slave */
    return false;
  }

  if (_hw->COUNT8.CTRLA.reg & TC_CTRLA_ENABLE)
  {
    /* Module must be disabled before initialization. Abort. */
    return false;
  }

  /* Set up the TC PWM out pin for channel 0 */
  if (_pwm_channel[0].enabled)
  {
    pinPeripheral(_pwm_channel[0].pin_out, (EPioType)_pwm_channel[0].pin_mux);
  }

  /* Set up the TC PWM out pin for channel 1 */
  if (_pwm_channel[1].enabled)
  {
    pinPeripheral(_pwm_channel[1].pin_out, (EPioType)_pwm_channel[1].pin_mux);
  }

#if defined(__SAMD51__)
	GCLK->PCHCTRL[inst_gclk_id[instance]].reg = GCLK_PCHCTRL_GEN_GCLK1_Val | (1 << GCLK_PCHCTRL_CHEN_Pos); //use clock generator 0
#else
  /* Enable the user interface clock in the PM */
  PM->APBCMASK.reg |= inst_pm_apbmask[instance];

  /* Enable the slave counter if counter_size is 32-bit */
  if ((_counter_size == TC_COUNTER_SIZE_32BIT))
  {
    /* Enable the user interface clock in the PM */
    PM->APBCMASK.reg |= inst_pm_apbmask[instance + 1];
  }

  /* Setup clock for module */
  GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | inst_gclk_id[instance]);
  while (GCLK->STATUS.bit.SYNCBUSY == 1);
#endif

  /* Set ctrla register */
  ctrla_tmp =
      (uint32_t)_counter_size |
#ifndef __SAMD51__
      (uint32_t)_wave_generation |
#endif
      (uint32_t)_clock_prescaler;

  /* Write configuration to register */
  while (tc_is_syncing(_hw));
  _hw->COUNT8.CTRLA.reg = ctrla_tmp;

#if defined(__SAMD51__)
  while (tc_is_syncing(_hw));
  _hw->COUNT8.WAVE.reg = _wave_generation;
#endif

  if (_count_direction)
  {
    ctrlbset_tmp |= TC_CTRLBSET_DIR;
  }

  /* Clear old ctrlb configuration */
  while (tc_is_syncing(_hw));
  _hw->COUNT8.CTRLBCLR.reg = 0xFF;

  /* Check if we actually need to go into a wait state. */
  if (ctrlbset_tmp)
  {
    while (tc_is_syncing(_hw));
    /* Write configuration to register */
    _hw->COUNT8.CTRLBSET.reg = ctrlbset_tmp;
  }

  /* Set ctrlc register*/
  ctrlc_tmp = _waveform_invert_output;

  /* Write configuration to register */
  while (tc_is_syncing(_hw));
#if defined(__SAMD51__)
  _hw->COUNT8.DRVCTRL.reg = ctrlc_tmp;
#else
  _hw->COUNT8.CTRLC.reg = ctrlc_tmp;
#endif

  /* Write configuration to register */
  while (tc_is_syncing(_hw));

  /* Switch for TC counter size  */
  switch (_counter_size)
  {
  case TC_COUNTER_SIZE_8BIT:
    while (tc_is_syncing(_hw));

    _hw->COUNT8.COUNT.reg =
        _counter_8_bit.value;

    while (tc_is_syncing(_hw));

    _hw->COUNT8.PER.reg =
        _counter_8_bit.period;

    while (tc_is_syncing(_hw));

    _hw->COUNT8.CC[0].reg =
        _counter_8_bit.compare_capture_channel[0];

    while (tc_is_syncing(_hw));

    _hw->COUNT8.CC[1].reg =
        _counter_8_bit.compare_capture_channel[1];

    return true;

  case TC_COUNTER_SIZE_16BIT:
    while (tc_is_syncing(_hw));

    _hw->COUNT16.COUNT.reg = _counter_16_bit.value;

    while (tc_is_syncing(_hw));

    _hw->COUNT16.CC[0].reg =
        _counter_16_bit.compare_capture_channel[0];

    while (tc_is_syncing(_hw));

    _hw->COUNT16.CC[1].reg =
        _counter_16_bit.compare_capture_channel[1];

    return true;

  case TC_COUNTER_SIZE_32BIT:
    while (tc_is_syncing(_hw));

    _hw->COUNT32.COUNT.reg = _counter_32_bit.value;

    while (tc_is_syncing(_hw));

    _hw->COUNT32.CC[0].reg =
        _counter_32_bit.compare_capture_channel[0];

    while (tc_is_syncing(_hw));

    _hw->COUNT32.CC[1].reg =
        _counter_32_bit.compare_capture_channel[1];

    return true;
  }

  return false;
}

boolean Adafruit_ZeroTimer::PWMout(boolean pwmout, uint8_t channum, uint8_t pin)
{
  Tc *const tc_modules[TC_INST_NUM] = TC_INSTS;

  if (channum > 1)
    return false; // we only support pwm #0 or #1

  if (!pwmout)
  {
    _pwm_channel[channum].enabled = false;
  }
  else
  {
    uint32_t pinout = 0xFFFF;
    uint32_t pinmux = 0xFFFF;

#if defined(__SAMD51__)
    if (_timernum == 3){
      if (channum == 0){
        if (pin == 10){
          pinout = PIN_PA18E_TC3_WO0;
          pinmux = MUX_PA18E_TC3_WO0;
        }
        if (pin == MISO){
          pinout = PIN_PA14E_TC3_WO0;
          pinmux = MUX_PA14E_TC3_WO0;
        }
      }
      if (channum == 1){
        if (pin == 11){
          pinout = PIN_PA19E_TC3_WO1;
          pinmux = MUX_PA19E_TC3_WO1;
        }
      }
    }

    if (_timernum == 4){
      if (channum == 0){
        if (pin == A4){
          pinout = PIN_PB08E_TC4_WO0;
          pinmux = MUX_PB08E_TC4_WO0;
        }
        if (pin == 7){
          pinout = PIN_PB12E_TC4_WO0;
          pinmux = MUX_PB12E_TC4_WO0;
        }
        if (pin == 1){
          pinout = PIN_PA22E_TC4_WO0;
          pinmux = MUX_PA22E_TC4_WO0;
        }
      }
      if (channum == 1){
        if (pin == A5){
          pinout = PIN_PB09E_TC4_WO1;
          pinmux = MUX_PB09E_TC4_WO1;
        }
        if (pin == 4){
          pinout = PIN_PB13E_TC4_WO1;
          pinmux = MUX_PB13E_TC4_WO1;
        }
        if (pin == 0){
          pinout = PIN_PA23E_TC4_WO1;
          pinmux = MUX_PA23E_TC4_WO1;
        }
      }
    }

    if (_timernum == 5){
      if (channum == 0){
        if (pin == 5){
          pinout = PIN_PB14E_TC5_WO0;
          pinmux = MUX_PB14E_TC5_WO0;
        }

      }
      if (channum == 1){
        if (pin == 6){
          pinout = PIN_PB15E_TC5_WO1;
          pinmux = MUX_PB15E_TC5_WO1;
        }
      }
    }
#else
    if (_timernum == 3)
    {
      if (channum == 0)
      {
        if (pin == 10)
        {
          pinout = PIN_PA18E_TC3_WO0;
          pinmux = MUX_PA18E_TC3_WO0;
        }
        if (pin == 2)
        {
          pinout = PIN_PA14E_TC3_WO0;
          pinmux = MUX_PA14E_TC3_WO0;
        }
      }
      if (channum == 1)
      {
        if (pin == 12)
        {
          pinout = PIN_PA19E_TC3_WO1;
          pinmux = MUX_PA19E_TC3_WO1;
        }
        if (pin == 5)
        {
          pinout = PIN_PA15E_TC3_WO1;
          pinmux = MUX_PA15E_TC3_WO1;
        }
      }
    }

    if (_timernum == 4)
    {
      if (channum == 0)
      {
        if (pin == 20)
        { // a.k.a SDA
          pinout = PIN_PA22E_TC4_WO0;
          pinmux = MUX_PA22E_TC4_WO0;
        }
#if defined(__SAMD21G18A__)
        if (pin == A1)
        {
          pinout = PIN_PB08E_TC4_WO0;
          pinmux = MUX_PB08E_TC4_WO0;
        }
#endif
      }
      if (channum == 1)
      {
        if (pin == 21)
        { // a.k.a SCL
          pinout = PIN_PA23E_TC4_WO1;
          pinmux = MUX_PA23E_TC4_WO1;
        }
#if defined(__SAMD21G18A__)
        if (pin == A2)
        {
          pinout = PIN_PB09E_TC4_WO1;
          pinmux = MUX_PB09E_TC4_WO1;
        }
#endif
      }
    }

    if (_timernum == 5)
    {
      if (channum == 0)
      {
#if defined(__SAMD21G18A__)
        if (pin == MOSI)
        {
          pinout = PIN_PB10E_TC5_WO0;
          pinmux = MUX_PB10E_TC5_WO0;
        }
#endif
        // only other option is D-, skip it!
      }
      if (channum == 1)
      {
#if defined(__SAMD21G18A__)
        if (pin == SCK)
        {
          pinout = PIN_PB11E_TC5_WO1;
          pinmux = MUX_PB11E_TC5_WO1;
        }
#endif
        // only other option is D+, skip it!
      }
    }
#endif

    if (pinout == 0xFFFF)
      return false;

    _pwm_channel[channum].enabled = true;
    _pwm_channel[channum].pin_out = pin;
    _pwm_channel[channum].pin_mux = pinmux;
  }

  // re-init
#if defined(__SAMD51__)
  _hw = tc_modules[_timernum];
#else
  _hw = tc_modules[_timernum - TC_INSTANCE_OFFSET];
#endif
  return tc_init();
}

void Adafruit_ZeroTimer::invertWave(uint8_t invert)
{
  _waveform_invert_output = invert;

  tc_init();
}

void Adafruit_ZeroTimer::configure(tc_clock_prescaler prescale, tc_counter_size countersize, tc_wave_generation wavegen, tc_count_direction countdir)
{
  Tc *const tc_modules[TC_INST_NUM] = TC_INSTS;

  if (_timernum > TC_INST_MAX_ID)
    return;

  _waveform_invert_output = false;

  _pwm_channel[0].enabled = false;
  _pwm_channel[0].pin_out = 0;
  _pwm_channel[0].pin_mux = 0;

  _pwm_channel[1].enabled = false;
  _pwm_channel[1].pin_out = 0;
  _pwm_channel[1].pin_mux = 0;

  _counter_16_bit.value = 0x0000;
  _counter_16_bit.compare_capture_channel[0] = 0x0000;
  _counter_16_bit.compare_capture_channel[1] = 0x0000;

  if (countersize == TC_COUNTER_SIZE_8BIT)
  {
    _counter_8_bit.period = 0xFF;
  }

  _clock_prescaler = prescale;
  _counter_size = countersize;
  _wave_generation = wavegen;
  _count_direction = countdir;

#if defined(__SAMD51__)
  _hw = tc_modules[_timernum];
#else
  _hw = tc_modules[_timernum - TC_INSTANCE_OFFSET];
#endif
  tc_init();
}

void Adafruit_ZeroTimer::setPeriodMatch(uint32_t period, uint32_t match, uint8_t channum)
{
  if (_counter_size == TC_COUNTER_SIZE_8BIT)
  {
    while (tc_is_syncing(_hw));
    _counter_8_bit.period = period;
    _hw->COUNT8.PER.reg = (uint8_t)period;
    setCompare(channum, match);
  }
  else if (_counter_size == TC_COUNTER_SIZE_16BIT)
  {
    setCompare(0, period);
    setCompare(1, match);
  }
  else if (_counter_size == TC_COUNTER_SIZE_32BIT)
  {
    setCompare(0, period);
    setCompare(1, match);
  }
}

void Adafruit_ZeroTimer::setCallback(boolean enable, tc_callback cb_type, void (*callback_func)(void))
{

  uint8_t instance = _timernum - TC_INSTANCE_OFFSET;
  if (enable)
  {
    /* Set the bit corresponding to the callback_type */
    if (cb_type == TC_CALLBACK_CC_CHANNEL0)
    {
      _register_callback_mask |= (uint32_t)TC_INTFLAG_MC(1) << (instance*TC_CALLBACK_BITS);
      _enable_callback_mask |= (uint32_t)TC_INTFLAG_MC(1) << (instance*TC_CALLBACK_BITS);
      __cb[4 + (instance*TC_CALLBACK_BITS)] = callback_func;
    }
    else if (cb_type == TC_CALLBACK_CC_CHANNEL1)
    {
      _register_callback_mask |= (uint32_t)TC_INTFLAG_MC(2) << (instance*TC_CALLBACK_BITS);
      _enable_callback_mask |= (uint32_t)TC_INTFLAG_MC(2) << (instance*TC_CALLBACK_BITS);
      __cb[5 + (instance*TC_CALLBACK_BITS)] = callback_func;
    }
    else
    {
      _register_callback_mask |= (1UL << cb_type) << (instance*TC_CALLBACK_BITS);
      _enable_callback_mask |= (1UL << cb_type) << (instance*TC_CALLBACK_BITS);
      __cb[cb_type + (instance*TC_CALLBACK_BITS)] = callback_func;
    }

    IRQn_Type _irqs[] = { TC3_IRQn, TC4_IRQn, TC5_IRQn };
    NVIC_ClearPendingIRQ(_irqs[instance]);
    NVIC_EnableIRQ(_irqs[instance]);

    /* Enable callback */
    if (cb_type == TC_CALLBACK_CC_CHANNEL0)
    {
      _hw->COUNT8.INTENSET.reg = TC_INTFLAG_MC(1);
    }
    else if (cb_type == TC_CALLBACK_CC_CHANNEL1)
    {
      _hw->COUNT8.INTENSET.reg = TC_INTFLAG_MC(2);
    }
    else
    {
      _hw->COUNT8.INTENSET.reg = (1UL << cb_type);
    }
  }
  else
  {
    /* Disable callback */
    if (cb_type == TC_CALLBACK_CC_CHANNEL0)
    {
      _hw->COUNT8.INTENCLR.reg = TC_INTFLAG_MC(1);
      _enable_callback_mask &= ~((uint32_t)(TC_INTFLAG_MC(1)) << (instance*TC_CALLBACK_BITS));
    }
    else if (cb_type == TC_CALLBACK_CC_CHANNEL1)
    {
      _hw->COUNT8.INTENCLR.reg = TC_INTFLAG_MC(2);
      _enable_callback_mask &= ~((uint32_t)(TC_INTFLAG_MC(2)) << (instance*TC_CALLBACK_BITS));
    }
    else
    {
      _hw->COUNT8.INTENCLR.reg = (1UL << cb_type);
      _enable_callback_mask &= ~((1UL << cb_type) << (instance*TC_CALLBACK_BITS));
    }
  }
}

void Adafruit_ZeroTimer::setCompare(uint8_t channum, uint32_t compare)
{
  if (channum > 1)
    return;

  // set the configuration
  if (_counter_size == TC_COUNTER_SIZE_8BIT)
  {
    _counter_8_bit.compare_capture_channel[channum] = compare;
  }
  else if (_counter_size == TC_COUNTER_SIZE_16BIT)
  {
    _counter_16_bit.compare_capture_channel[channum] = compare;
  }
  else if (_counter_size == TC_COUNTER_SIZE_32BIT)
  {
    _counter_32_bit.compare_capture_channel[channum] = compare;
  }

  // set it live
  while (tc_is_syncing(_hw));

  /* Read out based on the TC counter size */
  switch (_counter_size)
  {
  case TC_COUNTER_SIZE_8BIT:
    _hw->COUNT8.CC[channum].reg = (uint8_t)compare;
    break;

  case TC_COUNTER_SIZE_16BIT:
    _hw->COUNT16.CC[channum].reg = (uint16_t)compare;
    break;

  case TC_COUNTER_SIZE_32BIT:
    _hw->COUNT32.CC[channum].reg = (uint32_t)compare;
    break;
  }
}

void Adafruit_ZeroTimer::enable(boolean en)
{
  if (en)
  {
    while (tc_is_syncing(_hw));

    /* Enable TC module */
    _hw->COUNT8.CTRLA.reg |= TC_CTRLA_ENABLE;
  }
  else
  {
    while (tc_is_syncing(_hw));
    /* Disbale interrupt */
    _hw->COUNT8.INTENCLR.reg = TC_INTENCLR_MASK;
    /* Clear interrupt flag */
    _hw->COUNT8.INTFLAG.reg = TC_INTFLAG_MASK;

    /* Disable TC module */
    _hw->COUNT8.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  }
}

extern "C" {

//TODO: this could probably be optimized to be faster
static inline void __tc_cb_handler(uint32_t mask, uint32_t offset){
  mask &= _enable_callback_mask >> offset;
  int i = 0;
  uint32_t _active_cb = 1;
  while(mask){
    if(mask & _active_cb){
      __cb[i+offset](); //call the callback
    }
    mask &= ~_active_cb;
    i++;
    _active_cb <<= 1;
  }
}


void Adafruit_ZeroTimer::timerHandler(uint8_t timerNum){
  uint32_t mask;
  switch(timerNum){
    case 3:
      mask = TC3->COUNT8.INTFLAG.reg;
      __tc_cb_handler(mask, 0);
      TC3->COUNT8.INTFLAG.reg = 0b00111011; //clear
      break;
    case 4:
      mask = TC4->COUNT8.INTFLAG.reg;
      __tc_cb_handler(mask, TC_CALLBACK_BITS);
      TC4->COUNT8.INTFLAG.reg = 0b00111011; //clear
      break;
    case 5:
      mask = TC5->COUNT8.INTFLAG.reg;
      __tc_cb_handler(mask, TC_CALLBACK_BITS*2);
      TC5->COUNT8.INTFLAG.reg = 0b00111011; //clear
      break;
  }
}

};
