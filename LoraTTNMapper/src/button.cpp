#include "globals.h"
#include "button.h"

Button *b = NULL;

// Local logging tag
static const char TAG[] = __FILE__;

void button_init(int pin)
{
#ifdef BUTTON_PULLDOWN
  b = new Button(pin);
#else
  b = new ButtonPullup(pin);
#endif

  // attach events to the button

  b->setOnDoubleClicked([]() {});

  b->setOnClicked([]() {
    ESP_LOGI(TAG, "pressed");

     #if (USE_DISPLAY)
    if (PageNumber < PAGE_COUNT)
    {
      PageNumber++;
    }
    else
    {
      PageNumber = 1;
    }  
    showPage(PageNumber);
  #endif  
  });

  b->setOnHolding([]() {
    ESP_LOGI(TAG, "holding");
  });

  // attach interrupt to the button
  // attachInterrupt(digitalPinToInterrupt(pin), ButtonIRQ, CHANGE);
}

void readButton()
{
  b->update();
}