#if 0

#include "EasyGL.h"

GUITabbedPanel::GUITabbedPanel(const String& callback) : GUIRectangle(callback) {
	setTabButtonsBordersColor(0.0f, 0.0f, 0.0f);
	setTabButtonsColor(100, 150, 190);
	setFontScales(1.0f, 1.0f);
	setPosition(0.5, 0.5);
	upperPanel = new GUIPanel("tpup");
	lowerPanel = new GUIPanel("tpdown");
	mainPanel  = new GUIPanel("tpmp");
	mainPanel->setLayout(PL_YAXIS_LAYOUT);
	mainPanel->setInterval(0, 0);
	upperPanel->setVisibleBounds(true);
	upperPanel->enableBGColor(true);
	upperPanel->setLayout(PL_XAXIS_LAYOUT);
	upperPanel->setClipSize(0);
	lowerPanel->setVisibleBounds(true);
	lowerPanel->enableBGColor(true);
	lowerPanel->setInterval(0, 0);
	lowerPanel->setClipSize(0);
	mainPanel->addWidget(upperPanel);
	mainPanel->addWidget(lowerPanel);
	widgetType = WT_TABBED_PANEL;
	fontIndex  = 0;
	lockItems  = false;
}

GUITabbedPanel::~GUITabbedPanel() {
	deleteObject(mainPanel);
}

GUIEventListener*  GUITabbedPanel::getEventsListener() {
	return this;
}

void  GUITabbedPanel::setFontScales(const Tuple2f& scales) {
	setFontScales(scales.x, scales.y);
}

void  GUITabbedPanel::setFontScales(double wScale, double hScale) {
	fontScales.x = clamp(hScale, 0.01f, 20.0f);
	fontScales.y = clamp(wScale, 0.01f, 20.0f);
}

const Tuple2f&  GUITabbedPanel::getFontScales() {
	return fontScales;
}

bool GUITabbedPanel::LoadXMLSettings(const TiXmlElement* element) {
	if (!XMLArbiter::inspectElementInfo(element, "TabbedPanel"))
		return Logger::writeErrorLog("Need a TabbedPanel node in the xml file");

	Tuple3f     bordersColor         = upperPanel->getBordersColor();
	Tuple4f     bgColor              = upperPanel->getBGColor();
	int         count                = 0;
	fontScales.y  = XMLArbiter::fillComponents1f(element,   "hScale",    fontScales.y);
	fontScales.x  = XMLArbiter::fillComponents1f(element,   "wScale",    fontScales.x);
	fontIndex     = XMLArbiter::fillComponents1i(element,   "fontIndex", fontIndex);

	for (const TiXmlElement* outer = element->FirstChildElement();
		 outer;
		 outer = outer->NextSiblingElement() ) {
		const char* value = outer->Value();

		if (!value)
			continue;

		if (!strcmp(value, "Panel")) {
			GUIPanel* panel = new GUIPanel();

			if (!panel->LoadXMLSettings(outer) || !addPanel(panel))
				deleteObject(panel);

			continue;
		}

		if (!strcmp(value, "TabButtonsBordersColor"))
			XMLArbiter::fillComponents3f(outer, tabButtonsBordersColor);

		if (!strcmp(value, "TabButtonsColor"))
			XMLArbiter::fillComponents3f(outer, tabButtonsColor);

		if (!strcmp(value, "BordersColor"))
			XMLArbiter::fillComponents3f(outer, bordersColor);

		if (!strcmp(value, "BGColor"))
			XMLArbiter::fillComponents4f(outer, bgColor);
	}

	upperPanel->setBordersColor(bordersColor);
	lowerPanel->setBordersColor(bordersColor);
	upperPanel->setBGColor(bgColor);
	lowerPanel->setBGColor(bgColor);
	setFontScales(fontScales);
	return GUIRectangle::LoadXMLSettings(element) && lowerPanel->getWidgets().GetCount();
}

bool GUITabbedPanel::addPanel(GUIPanel* panel) {
	if (lockItems)
		return false;

	if (lowerPanel->addWidget(panel)) {
		char buffer[256];
		int  count = int(lowerPanel->getWidgets().GetCount()) - 1;
		sprintf(buffer, "count_tb %d", count);
		GUIButton* tabButton = new GUIButton(buffer);
		tabButton->setBordersColor(tabButtonsBordersColor);
		tabButton->setColor(tabButtonsColor);
		tabButton->setLabelString(panel->getCallbackString());
		tabButton->getLabel()->setFontIndex(fontIndex);
		tabButton->getLabel()->setScales(fontScales);
		tabButton->setMinAlpha((count == 0) ? 1.0f : 0.5f);
		upperPanel->addWidget(tabButton);
		panel->setVisible((count == 0));
		return true;
	}

	return false;
}

const   GUIPanel*  GUITabbedPanel::getCurrentPanel() const {
	const Widgets& widgets = lowerPanel->getWidgets();
	int   count  = int(widgets.GetCount());

	for (int t = 0; t < count; t++)
		if (widgets[t]->isVisible())
			return (GUIPanel*)widgets[t];

	return NULL;
}

const   int  GUITabbedPanel::getCurrentPanelIndex() const {
	const Widgets& widgets = lowerPanel->getWidgets();
	int   count  = int(widgets.GetCount());

	for (int t = 0; t < count; t++)
		if (widgets[t]->isVisible())
			return t;

	return -1;
}

void GUITabbedPanel::checkKeyboardEvents(KeyEvent evt, int extraInfo) {
	upperPanel->checkKeyboardEvents(evt, extraInfo);
	lowerPanel->checkKeyboardEvents(evt, extraInfo);
}

void GUITabbedPanel::checkMouseEvents(MouseEvent& evt, int extraInfo, bool rBits) {
	upperPanel->checkMouseEvents(evt, extraInfo, rBits);
	lowerPanel->checkMouseEvents(evt, extraInfo, rBits);
}

void GUITabbedPanel::actionPerformed(GUIEvent& evt) {
	const String& cbs             = evt.getCallbackString();
	GUIRectangle*      sourceRectangle = evt.getEventSource();
	int                widgetType      = sourceRectangle->getWidgetType();

	if ((widgetType == WT_BUTTON) && sourceRectangle->isClicked()) {
		const Widgets& widgets   = lowerPanel->getWidgets(),
					   &buttons   = upperPanel->getWidgets();
		int   target = atoi(sourceRectangle->getCallbackString().c_str() + 9),
			  count  = int(widgets.GetCount());

		for (int t = 0; t < count; t++) {
			widgets[t]->setVisible(t == target);
			((GUIAlphaElement*)buttons[t])->setMinAlpha((t == target) ? 1.0f : 0.5f);
		}
	}
}

GUIButton* GUITabbedPanel::getTabButton(int index) {
	const Widgets& widgets = upperPanel->getWidgets();
	int   count  = int(widgets.GetCount());

	for (int t = 0; t < count; t++)
		if (t == index)
			return (GUIButton*)widgets[t];

	return NULL;
}

void GUITabbedPanel::render(double clockTick) {
	if (!parent || !visible)
		return;

	lockItems = true;
	mainPanel->render(clockTick);
}

const void GUITabbedPanel::computeWindowBounds() {
	if (parent && update) {
		upperPanel->setParent(mainPanel);
		mainPanel->setAnchorPoint(getAnchorPoint());
		mainPanel->setPosition(getPosition());
		mainPanel->setParent(parent);
		mainPanel->forceUpdate(true);
		windowBounds = mainPanel->getWindowBounds();
		const Widgets& widgets     = lowerPanel->getWidgets();
		double           height      = 0;
		int             widgetCount = (int)widgets.GetCount();

		for (int i = 0; i < widgetCount; i++)
			height = height < widgets[i]->GetHeight() ? widgets[i]->GetHeight() : height;

		lowerPanel->setSizes(1.0f, height);
		upperPanel->setParent(this);
	}
}

void  GUITabbedPanel::setTabButtonsColor(const Tuple3f& color) {
	setTabButtonsColor(color.x, color.y, color.z);
}

void  GUITabbedPanel::setTabButtonsColor(double r, double g, double b) {
	tabButtonsColor.Set(clamp(r, 0.0f, 255.0f),
						clamp(g, 0.0f, 255.0f),
						clamp(b, 0.0f, 255.0f));
	tabButtonsColor.x /= (tabButtonsColor.x > 1.0) ? 255.0f : 1.0;
	tabButtonsColor.y /= (tabButtonsColor.y > 1.0) ? 255.0f : 1.0;
	tabButtonsColor.z /= (tabButtonsColor.z > 1.0) ? 255.0f : 1.0;
}

const Tuple3f& GUITabbedPanel::getTabButtonsColor() const {
	return tabButtonsColor;
}

void  GUITabbedPanel::setTabButtonsBordersColor(const Tuple3f& color) {
	setTabButtonsBordersColor(color.x, color.y, color.z);
}

void  GUITabbedPanel::setTabButtonsBordersColor(double r, double g, double b) {
	tabButtonsBordersColor.Set(clamp(r, 0.0f, 255.0f),
							   clamp(g, 0.0f, 255.0f),
							   clamp(b, 0.0f, 255.0f));
	tabButtonsBordersColor.x /= (tabButtonsBordersColor.x > 1.0) ? 255.0f : 1.0;
	tabButtonsBordersColor.y /= (tabButtonsBordersColor.y > 1.0) ? 255.0f : 1.0;
	tabButtonsBordersColor.z /= (tabButtonsBordersColor.z > 1.0) ? 255.0f : 1.0;
}

GUIPanel*  GUITabbedPanel::getLowerPanel() {
	return lowerPanel;
}

const Tuple3f& GUITabbedPanel::getTabButtonsBordersColor() const {
	return tabButtonsBordersColor;
}


#endif
