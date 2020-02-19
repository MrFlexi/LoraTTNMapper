#include "irqhandler.h"
#include "globals.h"

// Local logging tag
static const char TAG[] = __FILE__;

// irq handler task, handles all our application level interrupts
void irqHandler(void *pvParameters)
{

  configASSERT(((uint32_t)pvParameters) == 1); // FreeRTOS check

  uint32_t InterruptStatus;
  static bool mask_irq = false;

  // task remains in blocked state until it is notified by an irq
  for (;;)
  {
    xTaskNotifyWait(0x00,             // Don't clear any bits on entry
                    ULONG_MAX,        // Clear all bits on exit
                    &InterruptStatus, // Receives the notification value
                    portMAX_DELAY);   // wait forever


// button pressed?
#ifdef HAS_BUTTON
    if (InterruptStatus & BUTTON_IRQ)
      readButton();
#endif

#if (USE_INTERRUPTS)
// do we have a power event?
#if (HAS_PMU)
    if (InterruptStatus & PMU_IRQ_BIT)
      AXP192_event_handler();
#endif        
#endif

#if (USE_GYRO )
if (InterruptStatus & GYRO_IRQ_BIT)
  {
  LED_showDegree( 10 );      
  }
#endif

  }
}

#ifdef HAS_BUTTON
void IRAM_ATTR ButtonIRQ()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  xTaskNotifyFromISR(irqHandlerTask, BUTTON_IRQ, eSetBits,
                     &xHigherPriorityTaskWoken);

  if (xHigherPriorityTaskWoken)
    portYIELD_FROM_ISR();
}
#endif

#if (HAS_PMU)
void IRAM_ATTR PMU_IRQ()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  xTaskNotifyFromISR(irqHandlerTask, PMU_IRQ_BIT, eSetBits,
                     &xHigherPriorityTaskWoken);

  if (xHigherPriorityTaskWoken)
    portYIELD_FROM_ISR();
}
#endif

#if (USE_INTERRUPTS)
void IRAM_ATTR ADXL_IRQ()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  xTaskNotifyFromISR(irqHandlerTask, ADXL_IRQ_BIT, eSetBits,
                     &xHigherPriorityTaskWoken);

  if (xHigherPriorityTaskWoken)
    portYIELD_FROM_ISR();
}
#endif



#if (USE_GYRO)
void IRAM_ATTR GYRO_IRQ()
{
    mpuInterrupt = true;

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  xTaskNotifyFromISR(irqHandlerTask, GYRO_IRQ_BIT, eSetBits,
                     &xHigherPriorityTaskWoken);

  if (xHigherPriorityTaskWoken)
    portYIELD_FROM_ISR();
}
#endif


void mask_user_IRQ()
{
  xTaskNotify(irqHandlerTask, MASK_IRQ, eSetBits);
}

void unmask_user_IRQ() { xTaskNotify(irqHandlerTask, UNMASK_IRQ, eSetBits); }