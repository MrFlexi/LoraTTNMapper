#include "globals.h"
#include "button.h"

using namespace simplebutton;

static Button *b = NULL;

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

  b->setOnDoubleClicked([]() { });

  b->setOnClicked([]() {
    ESP_LOGI(TAG, "Button pressed");
  });

  b->setOnHolding([]() {

  });

  // attach interrupt to the button
  // attachInterrupt(digitalPinToInterrupt(pin), ButtonIRQ, CHANGE);
}

void readButton() { b->update(); }