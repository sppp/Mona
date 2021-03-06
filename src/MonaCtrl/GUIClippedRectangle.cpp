#if 0

#include "EasyGL.h"

GUIClippedRectangle::GUIClippedRectangle(int size) {
	setTextureRectangle(0, 0, 1, 1);
	setBordersColor(0.0f, 0.9f, 0.0f);
	setClipSize(size);
	setBGColor(0.413f, 0.7f, 0.52f, 0.75f);
	drawBackground = false;
	drawBounds     = false;
}

void  GUIClippedRectangle::setClipSize(int clipSizeArg) {
	clipSize = clamp(clipSizeArg, 0, 100);
}

int GUIClippedRectangle::getClipSize() {
	return clipSize;
}

void GUIClippedRectangle::setVisibleBounds(bool visibleArg) {
	drawBounds = visibleArg;
}

bool  GUIClippedRectangle::boundsVisible() {
	return drawBounds;
}

bool GUIClippedRectangle::loadXMLClippedRectangleInfo(const TiXmlElement* element) {
	if (!element)
		return false;

	const TiXmlElement* absolutePriority = XMLArbiter::getChildElementByName(element, "Texture");
	setVisibleBounds(XMLArbiter::analyzeBooleanAttr(element,  "drawBounds",     drawBounds));
	enableBGColor(XMLArbiter::analyzeBooleanAttr(element,     "drawBackground", drawBackground));
	setClipSize(XMLArbiter::fillComponents1i(element,         "clipSize",       clipSize));

	if (absolutePriority)
		texture.LoadXMLSettings(absolutePriority);

	for (const TiXmlElement* outer = element->FirstChildElement();
		 outer;
		 outer = outer->NextSiblingElement() ) {
		const char* value = outer->Value();

		if (!value)
			continue;

		if (!strcmp(value, "TextureRectangle"))
			setTextureRectangle(XMLArbiter::fillComponents4f(outer, textureRectangle));

		if (!strcmp(value, "BordersColor"))
			setBordersColor(XMLArbiter::fillComponents3f(outer, bordersColor));

		if (!strcmp(value, "BGColor"))
			setBGColor(XMLArbiter::fillComponents4f(outer, bgColor));
	}

	return true;
}

void  GUIClippedRectangle::setBGColor(const Tuple4f& color) {
	setBGColor(color.x, color.y, color.z, color.w);
}

void  GUIClippedRectangle::setBGColor(double r, double g, double b, double a) {
	bgColor.Set(clamp(r, 0.0f, 255.0f),
				clamp(g, 0.0f, 255.0f),
				clamp(b, 0.0f, 255.0f),
				clamp(a, 0.0f, 255.0f));
	bgColor.x /= (bgColor.x > 1.0) ? 255.0f : 1.0;
	bgColor.y /= (bgColor.y > 1.0) ? 255.0f : 1.0;
	bgColor.z /= (bgColor.z > 1.0) ? 255.0f : 1.0;
	bgColor.w /= (bgColor.w > 1.0) ? 255.0f : 1.0;
}

void  GUIClippedRectangle::setBordersColor(const Tuple3f& color) {
	setBordersColor(color.x, color.y, color.z);
}

const Tuple3f& GUIClippedRectangle::getBordersColor() {
	return bordersColor;
}

void  GUIClippedRectangle::setBordersColor(double r, double g, double b) {
	bordersColor.Set(clamp(r, 0.0f, 255.0f),
					 clamp(g, 0.0f, 255.0f),
					 clamp(b, 0.0f, 255.0f));
	bordersColor.x /= (bordersColor.x > 1.0) ? 255.0f : 1.0;
	bordersColor.y /= (bordersColor.y > 1.0) ? 255.0f : 1.0;
	bordersColor.z /= (bordersColor.z > 1.0) ? 255.0f : 1.0;
}

void GUIClippedRectangle::renderClippedBounds() {
	if (drawBackground || drawBounds) {
		glVertexPointer(2, GL_INT, 0, vertices[0]);
		glEnableClientState(GL_VERTEX_ARRAY);

		if (texture.getID() && drawBackground) {
			glTexCoordPointer(2, GL_FLOAT, 0, texCoords[0]) ;
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			texture.activate();
		}
	}

	if (drawBackground) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4fv(bgColor);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 7);
		glDisable(GL_BLEND);

		if (texture.getID()) {
			texture.deactivate();
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
	}

	if (drawBounds) {
		glColor3fv(bordersColor);
		glDrawArrays(GL_LINE_STRIP, 0, 7);
	}

	glColor3f(1, 1, 1);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void  GUIClippedRectangle::computeClippedBounds(const Tuple4i& windowBounds) {
	double yTexOffset    = float(clipSize) / (windowBounds.w - windowBounds.y),
		   xTexOffset    = float(clipSize) / (windowBounds.z - windowBounds.x);
	xTexOffset *= textureRectangle.z - textureRectangle.x;
	yTexOffset *= textureRectangle.w - textureRectangle.y;
	vertices[0].Set(windowBounds.x, windowBounds.y + clipSize);
	vertices[1].Set(windowBounds.x, windowBounds.w);
	vertices[2].Set(windowBounds.z - clipSize, windowBounds.w);
	vertices[3].Set(windowBounds.z, windowBounds.w - clipSize);
	vertices[4].Set(windowBounds.z, windowBounds.y);
	vertices[5].Set(windowBounds.x + clipSize, windowBounds.y);
	vertices[6].Set(windowBounds.x, windowBounds.y + clipSize);
	texCoords[0].Set(textureRectangle.x, textureRectangle.w - yTexOffset);
	texCoords[1].Set(textureRectangle.x, textureRectangle.y);
	texCoords[2].Set(textureRectangle.z - xTexOffset, textureRectangle.y             );
	texCoords[3].Set(textureRectangle.z, textureRectangle.y + yTexOffset);
	texCoords[4].Set(textureRectangle.z, textureRectangle.w);
	texCoords[5].Set(textureRectangle.x + xTexOffset, textureRectangle.w             );
	texCoords[6].Set(textureRectangle.x, textureRectangle.w - yTexOffset);
}

void  GUIClippedRectangle::enableBGColor(bool enable) {
	drawBackground = enable;
}

void GUIClippedRectangle::setTextureRectangle(const Tuple4f& tr) {
	setTextureRectangle(tr.x, tr.y, tr.z, tr.w);
}

void GUIClippedRectangle::setTextureRectangle(double x, double y, double z, double w) {
	if (x > 1.0f || y > 1.0f || z > 1.0f || w > 1.0f)
		if (texture.getID()) {
			x = clamp(x, 0.0f, float(texture.GetWidth() ));
			y = clamp(y, 0.0f, float(texture.GetHeight()));
			z = clamp(z, 0.0f, float(texture.GetWidth() ));
			w = clamp(w, 0.0f, float(texture.GetHeight()));
			x /= texture.GetWidth();
			z /= texture.GetWidth();
			w /= texture.GetHeight();
			y /= texture.GetHeight();
		}

	textureRectangle.Set(clamp(x, 0.0f, 1.0f),
						 clamp(y, 0.0f, 1.0f),
						 clamp(z, 0.0f, 1.0f),
						 clamp(w, 0.0f, 1.0f));
}

const  Tuple4f& GUIClippedRectangle::getTextureRectangle() {
	return textureRectangle;
}

Texture& GUIClippedRectangle::getTexture() {
	return texture;
}

void  GUIClippedRectangle::setTexture(const Texture& textureArg) {
	texture = textureArg;
}

bool  GUIClippedRectangle::isBGColorOn() {
	return drawBackground;
}

const Tuple4f& GUIClippedRectangle::getBGColor() {
	return bgColor;
}

const Tuple2i* GUIClippedRectangle::getVertices() const {
	return vertices;
}

const Tuple2f* GUIClippedRectangle::getTexCoords() const {
	return texCoords;
}


#endif
