#include "globals.h"
#include "button.h"

Button *b = NULL;

// Local logging tag
static const char TAG[] = "";

void button_init(int pin)
{
#ifdef BUTTON_PULLDOWN
  b = new Button(pin);
#else
  b = new ButtonPullup(pin);
#endif

  // attach events to the button

  b->setOnDoubleClicked([]() {
    ESP_LOGI(TAG, "Button double cklick");
    if (dataBuffer.data.pictureLoop)
          dataBuffer.data.pictureLoop = false;

    if (!dataBuffer.data.pictureLoop)
          dataBuffer.data.pictureLoop = true;

    

  });

  b->setOnClicked([]() {
    ESP_LOGI(TAG, "Button pressed");
  	t_moveDisplay();
  });

  b->setOnHolding([]() {
    ESP_LOGI(TAG, "Button Holding");
  });

}

void readButton()
{
  b->update();
}