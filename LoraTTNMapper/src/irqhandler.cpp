#include "irqhandler.h"
#include "globals.h"

// Local logging tag
static const char TAG[] = __FILE__;

uint32_t InterruptStatusRegister = 0;

//------------------------------------------------------------------------------------------
// // Irq handler RTOStask, handles all our application level interrupts
//------------------------------------------------------------------------------------------

void irqHandler(void *pvParameters)
{

  const TickType_t xDelay = 250 / portTICK_PERIOD_MS; // 100 ms = 10 times per second
  uint32_t InterruptStatus;
  static bool mask_irq = false;

  configASSERT(((uint32_t)pvParameters) == 1); // FreeRTOS check

  // task remains in blocked state until it is notified by an irq
  for (;;)
  {
    xTaskNotifyWait(0x00,             // Don't clear any bits on entry
                    ULONG_MAX,        // Clear all bits on exit
                    &InterruptStatus, // Receives the notification value
                    portMAX_DELAY);   // wait forever

#if (USE_INTERRUPTS)
//mqtt_send_irq();
      
// button pressed?
#if (USE_BUTTON)
    if (InterruptStatusRegister & BUTTON_IRQ)
      readButton();
#endif

// do we have a power event?
#if (HAS_PMU)
#if (USE_PMU_INTERRUPT)
    if (InterruptStatusRegister & PMU_IRQ_BIT)
    {
      AXP192_event_handler();
    }  
#endif
#endif

#if (USE_GYRO)
    if (InterruptStatusRegister & GYRO_IRQ_BIT)
    {
      gyro_handle_interrupt();
    }
#endif
#endif

    InterruptStatusRegister = 0;
    vTaskDelay(xDelay);
  }
}

//------------------------------------------------------------------------------------------
// Notify Interrupt Handler
//------------------------------------------------------------------------------------------

#if (USE_BUTTON)
void IRAM_ATTR ButtonIRQ()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  ESP_LOGI(TAG, "Button IRQ");

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
  InterruptStatusRegister |= PMU_IRQ_BIT;
  ESP_LOGI(TAG, "PMU IRQ");

  xTaskNotifyFromISR(irqHandlerTask, PMU_IRQ_BIT, eSetBits,
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