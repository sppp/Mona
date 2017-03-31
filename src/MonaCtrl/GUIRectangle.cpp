#include "EasyGL.h"

GUIRectangle::GUIRectangle(const String &callback)
{
  setCallbackString(callback);
  setSizes(5, 5);
  setPosition(0.5f, 0.5f);

  lastAction =          0;
  widgetType = WT_UNKNOWN;
  anchor     =  AT_CENTER;
  parent     =       NULL;
  z          =          1;

  mouseOver  =  false;
  released   =  false;
  pressed    =  false;
  clicked    =  false;
  focused    =  false;
  visible    =   true;
  active     =   true;
  update     =  false;
}

void GUIRectangle::setParent(GUIRectangle *parentArg)
{
  parent = parentArg;
  update = (parent != NULL);
}

GUIRectangle *GUIRectangle::getParent()
{
  return parent;
}

int   GUIRectangle::getWidgetType()
{ 
  return widgetType; 
}

void  GUIRectangle::forceUpdate(bool updateArg)
{
  update = updateArg;
}

bool GUIRectangle::loadXMLSettings(const TiXmlElement *element)
{
  if(!element)
    return false;

  const char *cbString = element->Attribute("callbackString");
              cbString = cbString ? cbString : element->Attribute("name");

  if(cbString)
    setCallbackString(cbString);
  else
    return Logger::writeErrorLog(String("Need a <name> or <callbackString> attribute, check ")
                                 + element->Value());

  if(element->Attribute("anchorPoint"))
    setAnchorPoint(element->Attribute("anchorPoint"));

  setVisible(XMLArbiter::analyzeBooleanAttr(element, "visible", true));
  setActive(XMLArbiter::analyzeBooleanAttr(element,  "active", true));

  for(const TiXmlElement *outer = element->FirstChildElement();	
      outer;
   	  outer = outer->NextSiblingElement() )
  {
    const char * value = outer->Value();

    if(value)
    {
      if(!strcmp(value, "Position"))
        setPosition(XMLArbiter::fillComponents2f(outer, position));
 
      if(!strcmp(value, "Sizes"))
        setSizes(XMLArbiter::fillComponents2f(outer, dimensions));
    }
  }

  return true;
}

void  GUIRectangle::enableGUITexture()
{
  if(parent)
    parent->enableGUITexture();
}

void  GUIRectangle::disableGUITexture()
{
  if(parent)
    parent->disableGUITexture();
}

void  GUIRectangle::setCallbackString(const String& callback)
{
  callbackString = callback;
}

const String & GUIRectangle::getCallbackString()
{
  return callbackString;
}  

void  GUIRectangle::setAnchorPoint(const String &anchorArg)
{
  if(!anchorArg.GetCount())
    return;

  if(anchorArg == "CENTER"  ){ anchor = AT_CENTER;   return; }
  if(anchorArg == "CORNERLU"){ anchor = AT_CORNERLU; return; }
  if(anchorArg == "CORNERRU"){ anchor = AT_CORNERRU; return; }
  if(anchorArg == "CORNERLD"){ anchor = AT_CORNERLD; return; }
  if(anchorArg == "CORNERRD"){ anchor = AT_CORNERRD; return; }
}

void  GUIRectangle::setAnchorPoint(int anchorArg)
{
  switch(anchorArg)
  {
    case AT_CORNERLU:
    case AT_CORNERRU:
    case AT_CORNERLD:
    case AT_CORNERRD:
      anchor = anchorArg;
    break;
    default:
      anchor = AT_CENTER;
  }
  update = true;
}

int   GUIRectangle::getAnchorPoint()
{
  return anchor;
}

GUITexCoordDescriptor *GUIRectangle::getTexCoordsInfo(int type)
{
  return !parent? NULL : parent->getTexCoordsInfo(type);
}

void GUIRectangle::setSizes(const Tuple2f &dimensions)
{
  setSizes(dimensions.x, dimensions.y);
}

void GUIRectangle::setSizes(float width, float height)
{
  dimensions.set(clamp(width , 0.01f, 2048.0f),
                 clamp(height, 0.01f, 2048.0f));
  update = true;
}

const Tuple2f &GUIRectangle::getSizes()
{
  return  dimensions;
}

void GUIRectangle::setPosition(const Tuple2f &scales)
{
  setPosition(scales.x, scales.y);
}

void GUIRectangle::setPosition(float xScale, float yScale)
{
  position.set(xScale, yScale);
  update = true;
}

const Tuple2f &GUIRectangle::getPosition()
{
  return position;
}

void GUIRectangle::setActive(bool activeArg)
{
  active = activeArg;
}

bool GUIRectangle::isActive()
{
  return active;
}

void GUIRectangle::setVisible(bool visibleArg)
{
  visible = visibleArg;
}

bool  GUIRectangle::isVisible()
{
  return visible;
}

bool  GUIRectangle::isAttached()
{
  return (parent != NULL);
}

int GUIRectangle::GetWidth()
{
  return windowBounds.z - windowBounds.x;
}

int GUIRectangle::GetHeight()
{
  return windowBounds.w - windowBounds.y;
}

const Tuple4i &GUIRectangle::getWindowBounds()
{
  computeWindowBounds();
  return windowBounds;
}

const void GUIRectangle::computeWindowBounds()
{
  if(parent && update)
  {
    const Tuple4i &parentBounds = parent->getWindowBounds();
    z  = parent->getZCoordinate() + 1;
    Tuple2f newSizes,
            newPosition;
 
    newSizes.x  = float(parentBounds.z - parentBounds.x);
    newSizes.y  = float(parentBounds.w - parentBounds.y);

    newPosition.x    = float(parentBounds.x);
    newPosition.y    = float(parentBounds.y);

    newPosition.x    = (position.x <= 1.0f) && (position.x >= 0.0f) ? 
                        newSizes.x*position.x + parentBounds.x :
                       (position.x < 0.0f) ?
                        parentBounds.z + position.x    :
                        position.x     + parentBounds.x;

    newPosition.y    = (position.y <= 1.0f) && (position.y >= 0.0f) ? 
                        newSizes.y*position.y + parentBounds.y :
                       (position.y < 0.0f) ?
                        parentBounds.w + position.y    :
                        position.y     + parentBounds.y;

    newSizes.x  = (dimensions.x <= 1.0f) ? newSizes.x*dimensions.x :
                                                dimensions.x;
    newSizes.y  = (dimensions.y <= 1.0f) ? newSizes.y*dimensions.y :
                                                dimensions.y;

    windowBounds.x = int(newPosition.x);
    windowBounds.y = int(newPosition.y);
    windowBounds.z = int(newPosition.x + newSizes.x);
    windowBounds.w = int(newPosition.y + newSizes.y);

    switch(anchor)
    {
      case AT_CORNERLD:
        windowBounds.y -= int(newSizes.y);
        windowBounds.w -= int(newSizes.y);
      break;

      case AT_CORNERRU:
        windowBounds.x -= int(newSizes.x);
        windowBounds.z -= int(newSizes.x);
      break;

      case AT_CORNERRD:
        windowBounds.y -= int(newSizes.y);
        windowBounds.w -= int(newSizes.y);
        windowBounds.x -= int(newSizes.x);
        windowBounds.z -= int(newSizes.x);
      break;

      case AT_CENTER:
        newSizes /= 2;

        windowBounds.y -= int(newSizes.y);
        windowBounds.w -= int(newSizes.y);
        windowBounds.x -= int(newSizes.x);
        windowBounds.z -= int(newSizes.x);
      break;
    }
  }
}

GUIEventListener *GUIRectangle::getEventsListener()
{
  return !parent ? NULL : parent->getEventsListener();
}

void GUIRectangle::checkKeyboardEvents(KeyEvent evt, int extraInfo){}
void GUIRectangle::checkMouseEvents(MouseEvent &newEvent, int extraInfo, bool bits)
{
  GUIEventListener *eventsListener = getEventsListener();
  bool              nRelease = pressed;

  mouseOver  = (newEvent.getY() >= windowBounds.y) &&
               (newEvent.getY() <= windowBounds.w) &&
               (newEvent.getX() >= windowBounds.x) &&
               (newEvent.getX() <= windowBounds.z);

  if(!mouseOver)
  {
    pressed = (extraInfo == ME_RELEASED) ? false : pressed;
    if(extraInfo == ME_PRESSED || extraInfo == ME_RELEASED)  focused = false;
    return;
  }

  if(extraInfo == ME_RELEASED && (lastAction == ME_CLICKED|| pressed))
  {
    clicked = true;
    pressed = false;
  }

  if(clicked) 
    focused = true;

  if(extraInfo == ME_CLICKED)
    pressed = true;

  lastAction = extraInfo;
  released   = !pressed && nRelease;

  if(eventsListener && eventDetected()){
    GUIEvent event = GUIEvent(this);
    eventsListener->actionPerformed(event);
  }

  if(!bits)
  {
    released = false;
    clicked  = false;
  }
}

bool GUIRectangle::eventDetected(){ return mouseOver ||
                                           released  ||
                                           focused   ||
                                           pressed   ||
                                           clicked;   }

bool GUIRectangle::isMouseOver()  { return mouseOver; }
bool GUIRectangle::isReleased()   { return released;  }
bool GUIRectangle::isFocused()    { return focused;   }
bool GUIRectangle::isPressed()    { return pressed;   }
bool GUIRectangle::isClicked()    { return clicked;   }

void GUIRectangle::setZCoordinate(int zArg)
{
  z = clamp(zArg, 0, 65553);
}

int    GUIRectangle::getZCoordinate()
{
  return z;
}

Tuple2i  GUIRectangle::getCenter()
{
  getWindowBounds();
  return Tuple2i((windowBounds.x + windowBounds.z)/2,
                 (windowBounds.y + windowBounds.w)/2);
}
