#include "irqhandler.h"

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

// do we have a power event?
#if (HAS_PMU)
    if (InterruptStatus & PMU_IRQ)
      AXP192_event_handler();
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

#ifdef HAS_PMU
void IRAM_ATTR PMUIRQ()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  xTaskNotifyFromISR(irqHandlerTask, PMU_IRQ, eSetBits,
                     &xHigherPriorityTaskWoken);

  if (xHigherPriorityTaskWoken)
    portYIELD_FROM_ISR();
}
#endif


#if  (USE_ADXL345)
void IRAM_ATTR ADXLIRQ()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  xTaskNotifyFromISR(irqHandlerTask, ADXL_IRQ, eSetBits,
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