// ==============================================================
//	This file is part of Glest (www.glest.org)
//
//	Copyright (C) 2001-2008 Martiño Figueroa
//
//	You can redistribute this code and/or modify it under
//	the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the
//	License, or (at your option) any later version
// ==============================================================

#include "components.h"

#include <algorithm>

#include "metrics.h"
#include "core_data.h"
#include "platform_util.h"
#include "util.h"
#include "conversion.h"
#include "lang.h"
#include "gen_uuid.h"
//#include <cxxabi.h>
#include "leak_dumper.h"

using namespace std;
using namespace Shared::Util;

namespace Glest{ namespace Game{

// =====================================================
//	class GraphicComponent
// =====================================================

float GraphicComponent::anim= 0.f;
float GraphicComponent::fade= 0.f;
const float GraphicComponent::animSpeed= 0.02f;
const float GraphicComponent::fadeSpeed= 0.01f;
// WHITE
Vec3f GraphicComponent::customTextColor = Vec3f(1.0,1.0,1.0);

std::map<std::string, std::map<std::string, GraphicComponent *> > GraphicComponent::registeredGraphicComponentList;

GraphicComponent::GraphicComponent(const std::string &containerName, const std::string &objName, bool registerControl) {
	this->containerName = containerName;
	this->instanceName = "";
	if(containerName == "" || objName == "") {
	    //char szBuf[8096]="";
	    //snprintf(szBuf,8096,"Control not properly registered Container [%s] Control [%s]\n",containerName.c_str(),objName.c_str());
		//throw megaglest_runtime_error(szBuf);
	}
	if(objName != "" && registerControl) {
		registerGraphicComponent(containerName,objName);
	}
	else {
		this->instanceName = objName;
	}
	this->fontCallbackName = objName + "_" + getNewUUD();
	CoreData::getInstance().registerFontChangedCallback(this->getFontCallbackName(), this);

	enabled  = true;
	editable = true;
	visible  = true;
	x = 0;
	y = 0;
	w = 0;
	h = 0;
	text = "";
	font = NULL;
	font3D = NULL;
	textNativeTranslation = "";
}

string GraphicComponent::getNewUUD() {
	char  uuid_str[38];
	get_uuid_string(uuid_str,sizeof(uuid_str));
	return string(uuid_str);
}

GraphicComponent::~GraphicComponent() {
	CoreData::getInstance().unRegisterFontChangedCallback(this->getFontCallbackName());
}

void GraphicComponent::clearRegisteredComponents(std::string containerName) {
	if(containerName == "") {
		GraphicComponent::registeredGraphicComponentList.clear();
	}
	else {
		GraphicComponent::registeredGraphicComponentList[containerName].clear();
	}
}

void GraphicComponent::clearRegisterGraphicComponent(std::string containerName, std::string objName) {
	GraphicComponent *obj = findRegisteredComponent(containerName, objName);
	if(obj) {
		GraphicComponent::registeredGraphicComponentList[containerName].erase(objName);
	}
}

void GraphicComponent::clearRegisterGraphicComponent(std::string containerName, std::vector<std::string> objNameList) {
	for(int idx = 0; idx < (int)objNameList.size(); ++idx) {
		GraphicComponent::clearRegisterGraphicComponent(containerName, objNameList[idx]);
	}
}

void GraphicComponent::registerGraphicComponent(std::string containerName, std::string objName) {
	// unregistered old name if we have been renamed
	if(this->getInstanceName() != "") {
		//printf("RENAME Register Callback detected calling: Control old [%s] new [%s]\n",this->getInstanceName().c_str(),objName.c_str());
		clearRegisterGraphicComponent(this->containerName, this->getInstanceName());
	}
	else {
		//printf("NEW Register Callback detected calling: Control container [%s] name [%s]\n",containerName.c_str(),objName.c_str());
	}

	if(containerName == "" || objName == "") {
	    //char szBuf[8096]="";
	    //snprintf(szBuf,8096,"Control not properly registered Container [%s] Control [%s]\n",this->containerName.c_str(),objName.c_str());
		//throw megaglest_runtime_error(szBuf);
	}

	this->containerName = containerName;
	this->instanceName = objName;
	registeredGraphicComponentList[containerName][objName] = this;
	//SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s Line: %d] registered [%s] [%s] count = %d\n",__FILE__,__FUNCTION__,__LINE__,containerName.c_str(),instanceName.c_str(),registeredGraphicComponentList[containerName].size());
}

void GraphicComponent::registerGraphicComponentOnlyFontCallbacks(std::string containerName, std::string objName) {
	if(this->getInstanceName() != "") {
		//printf("(FONT ONLY) RENAME Register Callback detected calling: Control old [%s] new [%s]\n",this->getInstanceName().c_str(),objName.c_str());
		clearRegisterGraphicComponent(this->containerName, this->getInstanceName());
	}
	else {
		//printf("(FONT ONLY) NEW Register Callback detected calling: Control container [%s] name [%s]\n",containerName.c_str(),objName.c_str());
	}

	if(containerName == "" || objName == "") {
	    //char szBuf[8096]="";
	    //snprintf(szBuf,8096,"Control not properly registered Container [%s] Control [%s]\n",this->containerName.c_str(),objName.c_str());
		//throw megaglest_runtime_error(szBuf);
	}

	this->containerName = containerName;
	this->instanceName = objName;
}

GraphicComponent * GraphicComponent::findRegisteredComponent(std::string containerName, std::string objName) {
	GraphicComponent *result = NULL;

	std::map<std::string, std::map<std::string, GraphicComponent *> >::iterator iterFind1 = GraphicComponent::registeredGraphicComponentList.find(containerName);
	if(iterFind1 != GraphicComponent::registeredGraphicComponentList.end()) {
		std::map<std::string, GraphicComponent *>::iterator iterFind2 = iterFind1->second.find(objName);
		if(iterFind2 != iterFind1->second.end()) {
			result = iterFind2->second;
		}
	}
	return result;
}

void GraphicComponent::applyAllCustomProperties(std::string containerName) {
	std::map<std::string, std::map<std::string, GraphicComponent *> >::iterator iterFind1 = GraphicComponent::registeredGraphicComponentList.find(containerName);
	if(iterFind1 != GraphicComponent::registeredGraphicComponentList.end()) {
		for(std::map<std::string, GraphicComponent *>::iterator iterFind2 = iterFind1->second.begin();
				iterFind2 != iterFind1->second.end(); ++iterFind2) {
			iterFind2->second->applyCustomProperties(containerName);
		}
	}
}

void GraphicComponent::applyCustomProperties(std::string containerName) {
	if(instanceName != "") {
		std::map<std::string, std::map<std::string, GraphicComponent *> >::iterator iterFind1 = GraphicComponent::registeredGraphicComponentList.find(containerName);
		if(iterFind1 != GraphicComponent::registeredGraphicComponentList.end()) {
			std::map<std::string, GraphicComponent *>::iterator iterFind2 = iterFind1->second.find(instanceName);
			if(iterFind2 != iterFind1->second.end()) {
				Config &config = Config::getInstance();

				//string languageToken = config.getString("Lang");
				string languageToken = Lang::getInstance().getLanguage();

				//if(dynamic_cast<GraphicButton *>(iterFind2->second) != NULL) {
				GraphicComponent *ctl = dynamic_cast<GraphicComponent *>(iterFind2->second);

				// First check default overrides
				ctl->x = config.getInt(containerName + "_" + iterFind2->first + "_x",intToStr(ctl->x).c_str());
				ctl->y = config.getInt(containerName + "_" + iterFind2->first + "_y",intToStr(ctl->y).c_str());
				ctl->w = config.getInt(containerName + "_" + iterFind2->first + "_w",intToStr(ctl->w).c_str());
				ctl->h = config.getInt(containerName + "_" + iterFind2->first + "_h",intToStr(ctl->h).c_str());
				ctl->visible = config.getBool(containerName + "_" + iterFind2->first + "_visible",boolToStr(ctl->visible).c_str());

				// Now check language specific overrides
				ctl->x = config.getInt(containerName + "_" + iterFind2->first + "_x_" + languageToken, intToStr(ctl->x).c_str());
				ctl->y = config.getInt(containerName + "_" + iterFind2->first + "_y_" + languageToken, intToStr(ctl->y).c_str());
				ctl->w = config.getInt(containerName + "_" + iterFind2->first + "_w_" + languageToken, intToStr(ctl->w).c_str());
				ctl->h = config.getInt(containerName + "_" + iterFind2->first + "_h_" + languageToken, intToStr(ctl->h).c_str());
				ctl->visible = config.getBool(containerName + "_" + iterFind2->first + "_visible_" + languageToken,boolToStr(ctl->visible).c_str());
			}
		}
	}
}

bool GraphicComponent::saveAllCustomProperties(std::string containerName) {
	SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s Line: %d] registered [%s] count = %d\n",__FILE__,__FUNCTION__,__LINE__,containerName.c_str(),registeredGraphicComponentList[containerName].size());

	bool foundPropertiesToSave = false;
	std::map<std::string, std::map<std::string, GraphicComponent *> >::iterator iterFind1 = GraphicComponent::registeredGraphicComponentList.find(containerName);
	if(iterFind1 != GraphicComponent::registeredGraphicComponentList.end()) {
		for(std::map<std::string, GraphicComponent *>::iterator iterFind2 = iterFind1->second.begin();
				iterFind2 != iterFind1->second.end(); ++iterFind2) {
			bool saved = iterFind2->second->saveCustomProperties(containerName);
			foundPropertiesToSave = (saved || foundPropertiesToSave);
		}
	}

	if(foundPropertiesToSave == true) {
		Config &config = Config::getInstance();
		config.save();
	}

	return foundPropertiesToSave;
}

bool GraphicComponent::saveCustomProperties(std::string containerName) {
	bool savedChange = false;
	if(instanceName != "") {

		SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s Line: %d] looking for [%s] [%s]\n",__FILE__,__FUNCTION__,__LINE__,containerName.c_str(),instanceName.c_str());

		std::map<std::string, std::map<std::string, GraphicComponent *> >::iterator iterFind1 = GraphicComponent::registeredGraphicComponentList.find(containerName);
		if(iterFind1 != GraphicComponent::registeredGraphicComponentList.end()) {

			SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s Line: %d] looking for [%s]\n",__FILE__,__FUNCTION__,__LINE__,instanceName.c_str());

			std::map<std::string, GraphicComponent *>::iterator iterFind2 = iterFind1->second.find(instanceName);
			if(iterFind2 != iterFind1->second.end()) {

				SystemFlags::OutputDebug(SystemFlags::debugSystem,"In [%s::%s Line: %d] FOUND [%s]\n",__FILE__,__FUNCTION__,__LINE__,instanceName.c_str());

				Config &config = Config::getInstance();

				//string languageToken = config.getString("Lang");

				//if(dynamic_cast<GraphicButton *>(iterFind2->second) != NULL) {
				GraphicComponent *ctl = dynamic_cast<GraphicComponent *>(iterFind2->second);

				// First check default overrides
				config.setInt(containerName + "_" + iterFind2->first + "_x",ctl->x);
				config.setInt(containerName + "_" + iterFind2->first + "_y",ctl->y);
				config.setInt(containerName + "_" + iterFind2->first + "_w",ctl->w);
				config.setInt(containerName + "_" + iterFind2->first + "_h",ctl->h);

				savedChange = true;
				// Now check language specific overrides
				//ctl->x = config.getInt(containerName + "_" + iterFind2->first + "_x_" + languageToken, intToStr(ctl->x).c_str());
				//ctl->y = config.getInt(containerName + "_" + iterFind2->first + "_y_" + languageToken, intToStr(ctl->y).c_str());
				//ctl->w = config.getInt(containerName + "_" + iterFind2->first + "_w_" + languageToken, intToStr(ctl->w).c_str());
				//ctl->h = config.getInt(containerName + "_" + iterFind2->first + "_h_" + languageToken, intToStr(ctl->h).c_str());

				//}
			}
		}
	}

	return savedChange;
}

void GraphicComponent::setFont(Font2D *font) { 
	this->font = font;
	if (this->font != NULL) {
		this->font2DUniqueId = font->getFontUniqueId();
	}
	else {
		this->font2DUniqueId = "";
	}
}

void GraphicComponent::setFont3D(Font3D *font) { 
	this->font3D = font;
	if (this->font3D != NULL) {
		this->font3DUniqueId = font->getFontUniqueId();
	}
	else {
		this->font3DUniqueId = "";
	}
}

void GraphicComponent::FontChangedCallback(std::string fontUniqueId, Font *font) {
	//printf("In FontChanged for [%s] font [%p] Control 2D [%s] 3D [%s]\n", fontUniqueId.c_str(),font,this->font2DUniqueId.c_str(),this->font3DUniqueId.c_str());
	if (fontUniqueId != "") {
		if (fontUniqueId == this->font2DUniqueId) {
			if (font != NULL) {
				this->font = (Font2D *)font;
			}
			else {
				this->font = NULL;
			}
		}
		else if (fontUniqueId == this->font3DUniqueId) {
			if (font != NULL) {
				this->font3D = (Font3D *)font;
			}
			else {
				this->font3D = NULL;
			}
		}
	}
}

void GraphicComponent::reloadFonts() {
	setFont(CoreData::getInstance().getMenuFontNormal());
	setFont3D(CoreData::getInstance().getMenuFontNormal3D());
}

void GraphicComponent::reloadFontsForRegisterGraphicComponents(std::string containerName) {
	std::map<std::string, std::map<std::string, GraphicComponent *> >::iterator iterFind1 = GraphicComponent::registeredGraphicComponentList.find(containerName);
	if(iterFind1 != GraphicComponent::registeredGraphicComponentList.end()) {
		for(std::map<std::string, GraphicComponent *>::iterator iterFind2 = iterFind1->second.begin();
				iterFind2 != iterFind1->second.end(); ++iterFind2) {
			GraphicComponent *ctl = dynamic_cast<GraphicComponent *>(iterFind2->second);
			if(ctl) {
				ctl->reloadFonts();
			}
		}
	}
}

void GraphicComponent::init(int x, int y, int w, int h) {
    this->x= x;
    this->y= y;
    this->w= w;
    this->h= h;
    reloadFonts();
	enabled= true;
}
bool GraphicComponent::eventMouseWheel(int x, int y,int zDelta){
	return false;
}

bool GraphicComponent::mouseMove(int x, int y) {
	if(this->getVisible() == false) {
		return false;
	}

    return
        x > this->x &&
        y > this->y &&
        x < this->x + w &&
        y < this->y + h;
}

bool GraphicComponent::mouseClick(int x, int y){
	if(getVisible() && getEnabled() && getEditable())
    	return mouseMove(x, y);
    else
    	return false;
}

void GraphicComponent::update(){
	fade+= fadeSpeed;
	anim+= animSpeed;
	if(fade>1.f) fade= 1.f;
	if(anim>1.f) anim= 0.f;
}

void GraphicComponent::resetFade(){
	fade= 0.f;
}

// =====================================================
//	class GraphicLabel
// =====================================================

const int GraphicLabel::defH= 20;
const int GraphicLabel::defW= 70;

GraphicLabel::GraphicLabel(const std::string &containerName, const std::string &objName, bool registerControl) :
		GraphicComponent(containerName, objName, registerControl) {
	centered = false;
	wordWrap = false;
	centeredW = -1;
	centeredH = 1;
	editable = false;
	editModeEnabled = false;
	maxEditWidth = -1;
	maxEditRenderWidth = -1;
	renderBackground = false;
	backgroundColor=Vec4f(0.2f,0.2f,0.2f,0.6f);
	isPassword = false;
	texture = NULL;
}

void GraphicLabel::init(int x, int y, int w, int h, bool centered, Vec3f textColor, bool wordWrap) {
	GraphicComponent::init(x, y, w, h);
	this->centered= centered;
	this->textColor=textColor;
	this->wordWrap = wordWrap;
}

bool GraphicLabel::mouseMove(int x, int y) {
	if(this->getVisible() == false) {
		return false;
	}

	int useWidth = w;
	if(text.length() > 0 && font3D != NULL) {
		float lineWidth = (font3D->getTextHandler()->Advance(text.c_str()) * Shared::Graphics::Font::scaleFontValue);
		useWidth = (int)lineWidth;
	}

	if(editable && useWidth<getMaxEditRenderWidth()){
		useWidth = getMaxEditRenderWidth();
	}

    return
        x > this->x &&
        y > this->y &&
        x < this->x + useWidth &&
        y < this->y + h;
}

bool GraphicLabel::getCenteredW() const {
	bool result = (centered || centeredW == 1);
	return result;
}
//void GraphicLabel::setCenteredW(bool centered) {
//	centeredW = (centered ? 1 : 0);
//}

bool GraphicLabel::getCenteredH() const {
	bool result = (centered || centeredH == 1);
	return result;
}
//void GraphicLabel::setCenteredH(bool centered) {
//	centeredH = (centered ? 1 : 0);
//}

// =====================================================
//	class GraphicButton
// =====================================================

const int GraphicButton::defH= 22;
const int GraphicButton::defW= 90;

GraphicButton::GraphicButton(const std::string &containerName, const std::string &objName, bool registerControl) :
	GraphicComponent(containerName,objName,registerControl) {

	lighted = false;
	alwaysLighted = false;
	useCustomTexture = false;
	customTexture = NULL;
}

void GraphicButton::init(int x, int y, int w, int h){
	GraphicComponent::init(x, y, w, h);
    lighted= false;
}

bool GraphicButton::mouseMove(int x, int y){
	if(this->getVisible() == false) {
		return false;
	}

	bool b= GraphicComponent::mouseMove(x, y);
    lighted= b;
    return b;
}

// =====================================================
//	class GraphicListBox
// =====================================================

const int GraphicListBox::defH= 22;
const int GraphicListBox::defW= 140;

GraphicListBox::GraphicListBox(const std::string &containerName, const std::string &objName)
: GraphicComponent(containerName, objName), graphButton1(containerName, objName + "_button1"),
											graphButton2(containerName, objName + "_button2") {
    selectedItemIndex = 0;
    lighted = false;
    leftControlled = false;
}

void GraphicListBox::init(int x, int y, int w, int h, Vec3f textColor){
	GraphicComponent::init(x, y, w, h);

	this->textColor=textColor;
	graphButton1.init(x, y, h, h);
    graphButton2.init(x+w-h, y, h, h);
    graphButton1.setText("<");
    graphButton2.setText(">");
    selectedItemIndex=-1;
    lighted=false;
}

const string & GraphicListBox::getTextNativeTranslation() {
	if(this->translated_items.empty() == true ||
			this->selectedItemIndex < 0 ||
			this->selectedItemIndex >= (int)this->translated_items.size() ||
			this->items.size() != this->translated_items.size()) {
		return this->text;
	}
	else {
		return this->translated_items[this->selectedItemIndex];
	}
}

//queryes
void GraphicListBox::pushBackItem(string item, string translated_item){
    items.push_back(item);
    translated_items.push_back(translated_item);
    setSelectedItemIndex(0);
}

void GraphicListBox::clearItems(){
    items.clear();
    translated_items.clear();
	selectedItemIndex=-1;
	setText("");
}


void GraphicListBox::setItems(const vector<string> &items, const vector<string> translated_items){
    this->items= items;
    this->translated_items = translated_items;
    if(items.empty() == false) {
    	setSelectedItemIndex(0);
    }
    else {
    	selectedItemIndex=-1;
    	setText("");
    }
}

void GraphicListBox::setSelectedItemIndex(int index, bool errorOnMissing){
	if(errorOnMissing == true && (index < 0 || index >= (int)items.size())) {
	    char szBuf[8096]="";
	    snprintf(szBuf,8096,"Index not found in listbox name: [%s] value index: %d size: %lu",this->instanceName.c_str(),index,(unsigned long)items.size());
		throw megaglest_runtime_error(szBuf);
	}
    selectedItemIndex= index;
    setText(getSelectedItem());
}

void GraphicListBox::setLeftControlled(bool leftControlled) {
	if(this->leftControlled!=leftControlled){
		this->leftControlled= leftControlled;
		if(leftControlled==true) {
			graphButton2.setX(x+graphButton1.getW()-4);
			graphButton2.setH(graphButton2.getH()-4);
			graphButton2.setW(graphButton2.getW()-4);
			graphButton1.setH(graphButton1.getH()-4);
			graphButton1.setW(graphButton1.getW()-4);
			graphButton2.setY(graphButton2.getY()+2);
			graphButton1.setY(graphButton1.getY()+2);
		}
		else {
			graphButton2.setX(x+w-graphButton2.getW()+4);
			graphButton2.setH(graphButton2.getH()+4);
			graphButton2.setW(graphButton2.getW()+4);
			graphButton1.setH(graphButton1.getH()+4);
			graphButton1.setW(graphButton1.getW()+4);
			graphButton2.setY(graphButton2.getY()-2);
			graphButton1.setY(graphButton1.getY()-2);
		}
	}
}

void GraphicListBox::setX(int x) {
	this->x= x;
	graphButton1.setX(x);
	if(leftControlled==true) {
		graphButton2.setX(x+graphButton1.getW());
	}
	else {
		graphButton2.setX(x+w-graphButton2.getW());
	}
}

void GraphicListBox::setY(int y) {
	this->y= y;
	graphButton1.setY(y);
	graphButton2.setY(y);
}

void GraphicListBox::setEditable(bool editable){
    graphButton1.setEditable(editable);
    graphButton2.setEditable(editable);
    GraphicComponent::setEditable(editable);
}

bool GraphicListBox::hasItem(string item) const {
	bool result = false;
	vector<string>::const_iterator iter= find(items.begin(), items.end(), item);
	if(iter != items.end()) {
		result = true;
	}

	return result;
}

void GraphicListBox::setSelectedItem(string item, bool errorOnMissing){
	vector<string>::iterator iter;

    iter= find(items.begin(), items.end(), item);

	if(iter==items.end()) {
		if(errorOnMissing == true) {
			for(int idx = 0; idx < (int)items.size(); idx++) {
				SystemFlags::OutputDebug(SystemFlags::debugError,"In [%s::%s Line: %d]\ninstanceName [%s] idx = %d items[idx] = [%s]\n",extractFileFromDirectoryPath(__FILE__).c_str(),__FUNCTION__,__LINE__,instanceName.c_str(),idx,items[idx].c_str());
			}

		    char szBuf[8096]="";
		    snprintf(szBuf,8096,"Value not found in listbox name: [%s] value: %s",this->instanceName.c_str(),item.c_str());
			throw megaglest_runtime_error(szBuf);
		}
	}
	else {
		setSelectedItemIndex(iter-items.begin());
	}

}

bool GraphicListBox::mouseMove(int x, int y){
	if(this->getVisible() == false) {
		return false;
	}

	return
        graphButton1.mouseMove(x, y) ||
        graphButton2.mouseMove(x, y);
}

bool GraphicListBox::mouseClick(int x, int y,string advanceToItemStartingWith) {
	if(this->getVisible() == false) {
		return false;
	}

	if(!items.empty()) {
		bool b1= graphButton1.mouseClick(x, y);
		bool b2= graphButton2.mouseClick(x, y);

		if(b1) {
			bool bFound = false;
			if(advanceToItemStartingWith != "") {
				for(int i = selectedItemIndex - 1; i >= 0; --i) {
					string item = items[i];
					if((int)translated_items.size() > i) item = translated_items[i];
					if(StartsWith(toLower(item),toLower(advanceToItemStartingWith)) == true) {
						bFound = true;
						selectedItemIndex = i;
						break;
					}
				}
				if(bFound == false) {
					for(int i = (int)items.size() - 1; i >= selectedItemIndex; --i) {
						string item = items[i];
						if((int)translated_items.size() > i) item = translated_items[i];
						//printf("Trying to match [%s] with item [%s]\n",advanceToItemStartingWith.c_str(),item.c_str());
						if(StartsWith(toLower(item),toLower(advanceToItemStartingWith)) == true) {
							bFound = true;
							selectedItemIndex = i;
							break;
						}
					}
				}
			}
			if(bFound == false) {
				selectedItemIndex--;
			}
			if(selectedItemIndex<0){
				selectedItemIndex = (int)items.size()-1;
			}
		}
		else if(b2) {
			bool bFound = false;
			if(advanceToItemStartingWith != "") {
				for(int i = selectedItemIndex + 1; i < (int)items.size(); ++i) {
					string item = items[i];
					if((int)translated_items.size() > i) item = translated_items[i];
					//printf("Trying to match [%s] with item [%s]\n",advanceToItemStartingWith.c_str(),item.c_str());
					if(StartsWith(toLower(item),toLower(advanceToItemStartingWith)) == true) {
						bFound = true;
						selectedItemIndex = i;
						break;
					}
				}
				if(bFound == false) {
					for(int i = 0; i <= selectedItemIndex; ++i) {
						string item = items[i];
						if((int)translated_items.size() > i) item = translated_items[i];
						//printf("Trying to match [%s] with item [%s]\n",advanceToItemStartingWith.c_str(),item.c_str());
						if(StartsWith(toLower(item),toLower(advanceToItemStartingWith)) == true) {
							bFound = true;
							selectedItemIndex = i;
							break;
						}
					}
				}
			}
			if(bFound == false) {
				selectedItemIndex++;
			}
			if(selectedItemIndex >= (int)items.size()) {
				selectedItemIndex=0;
			}
		}
		setText(getSelectedItem());

		return b1 || b2;
	}
	return false;
}

// =====================================================
//	class GraphicComboBox
// =====================================================

const int GraphicComboBox::defH= 22;
const int GraphicComboBox::defW= 140;

GraphicComboBox::GraphicComboBox(const std::string &containerName, const std::string &objName)
: GraphicComponent(containerName, objName), dropDownButton(containerName, objName + "_button") {
    selectedItemIndex = 0;
    lighted = false;
}

void GraphicComboBox::init(int x, int y, int w, int h, Vec3f textColor){
	GraphicComponent::init(x, y, w, h);

	this->textColor=textColor;
	dropDownButton.init(x+w-h, y, h, h);
    dropDownButton.setText("v");

    popupLineCount=15;
    popupButtonHeight=20;

    int scrollBarLength=popupLineCount*popupButtonHeight;
    int popupYpos=y+h;
    if( y>200) popupYpos=y-scrollBarLength;
    scrollBar.init(x+w-h,popupYpos,false,scrollBarLength,20);
	scrollBar.setVisibleSize(popupLineCount);
	scrollBar.setVisibleStart(0);
	scrollBar.setLighted(false);
	scrollBar.setVisible(true);
	setPopupLineCount(popupLineCount);

    selectedItemIndex=-1;
    popupShowing=false;
    lighted=false;
    preselectedItemIndex=-1;
}

void  GraphicComboBox::setPopupLineCount(int popupLineCount) {
		this->popupLineCount = popupLineCount;
		int scrollBarLength=popupLineCount*popupButtonHeight;
	    int popupYpos=getY()+getH();
	    if( y>200) popupYpos=y-scrollBarLength;
	    scrollBar.setY(popupYpos);
		scrollBar.setLength(scrollBarLength);
		scrollBar.setVisibleSize(popupLineCount);
	}

const string & GraphicComboBox::getTextNativeTranslation(int index) {
	if(this->translated_items.empty() == true ||
			index < 0 ||
			index >= (int)this->translated_items.size() ||
			this->items.size() != this->translated_items.size()) {
		return this->text;
	}
	else {
		return this->translated_items[index];
	}
}

const string & GraphicComboBox::getTextNativeTranslation() {
	return getTextNativeTranslation(this->selectedItemIndex);
}

//queryes
void GraphicComboBox::pushBackItem(string item, string translated_item){
    items.push_back(item);
    translated_items.push_back(translated_item);
    setSelectedItemIndex(0);
    createButton(item);
}

void GraphicComboBox::clearItems(){
	clearButtons();
    items.clear();
    translated_items.clear();
	selectedItemIndex=-1;
	setText("");
}

GraphicButton* GraphicComboBox::createButton(string item) {
	GraphicButton *button = new GraphicButton();
	button->init(getX(), scrollBar.getY(), getW()-scrollBar.getW(), getPopupButtonHeight());
	button->setText(item);
	button->setUseCustomTexture(true);
	button->setCustomTexture(CoreData::getInstance().getCustomTexture());
	popupButtons.push_back(button);
	scrollBar.setElementCount((int)popupButtons.size());
	layoutButtons();
	return button;
}

void GraphicComboBox::layoutButtons() {
	if (scrollBar.getElementCount() != 0) {
		int keyButtonsYBase=scrollBar.getY()+scrollBar.getLength();;
		for (int i = scrollBar.getVisibleStart();
				i <= scrollBar.getVisibleEnd(); ++i) {
			if(i >= (int)popupButtons.size()) {
				char szBuf[8096]="";
				snprintf(szBuf,8096,"i >= popupButtons.size(), i = %d, popupButtons.size() = %d",i,(int)popupButtons.size());
				throw megaglest_runtime_error(szBuf);
			}
			popupButtons[i]->setY(keyButtonsYBase - getPopupButtonHeight() * (i+1
					- scrollBar.getVisibleStart()));
		}
	}

}

void GraphicComboBox::clearButtons() {
	while(!popupButtons.empty()) {
		delete popupButtons.back();
		popupButtons.pop_back();
	}
	scrollBar.setElementCount(0);
}

void GraphicComboBox::setItems(const vector<string> &items, const vector<string> translated_items){
	clearItems();
    this->items= items;
    this->translated_items = translated_items;
    if(items.empty() == false) {
    	setSelectedItemIndex(0);
    }
    else {
    	selectedItemIndex=-1;
    	setText("");
    }
	for(int idx = 0; idx < (int)items.size(); idx++) {
		createButton(this->items[idx]);
	}
}

void GraphicComboBox::setSelectedItemIndex(int index, bool errorOnMissing){
	if(errorOnMissing == true && (index < 0 || index >= (int)items.size())) {
	    char szBuf[8096]="";
	    snprintf(szBuf,8096,"Index not found in listbox name: [%s] value index: %d size: %lu",this->instanceName.c_str(),index,(unsigned long)items.size());
		throw megaglest_runtime_error(szBuf);
	}
    selectedItemIndex= index;
    preselectedItemIndex= index;
    setText(getSelectedItem());
}


void GraphicComboBox::setX(int x) {
	this->x= x;
	dropDownButton.setX(x+w);
}

void GraphicComboBox::setY(int y) {
	this->y= y;
	dropDownButton.setY(y);
}

void GraphicComboBox::setEditable(bool editable){
    dropDownButton.setEditable(editable);
    GraphicComponent::setEditable(editable);
}

bool GraphicComboBox::hasItem(string item) const {
	bool result = false;
	vector<string>::const_iterator iter= find(items.begin(), items.end(), item);
	if(iter != items.end()) {
		result = true;
	}

	return result;
}

void GraphicComboBox::setSelectedItem(string item, bool errorOnMissing){
	vector<string>::iterator iter;

    iter= find(items.begin(), items.end(), item);

	if(iter==items.end()) {
		if(errorOnMissing == true) {
			for(int idx = 0; idx < (int)items.size(); idx++) {
				SystemFlags::OutputDebug(SystemFlags::debugError,"In [%s::%s Line: %d]\ninstanceName [%s] idx = %d items[idx] = [%s]\n",extractFileFromDirectoryPath(__FILE__).c_str(),__FUNCTION__,__LINE__,instanceName.c_str(),idx,items[idx].c_str());
			}

		    char szBuf[8096]="";
		    snprintf(szBuf,8096,"Value not found in listbox name: [%s] value: %s",this->instanceName.c_str(),item.c_str());
			throw megaglest_runtime_error(szBuf);
		}
	}
	else {
		setSelectedItemIndex(iter-items.begin());
	}

}

void GraphicComboBox::togglePopupVisibility(){
	if( popupShowing == true ){
		popupShowing=false;
		scrollBar.setVisible(false);
	}
	else {
		popupShowing=true;
		scrollBar.setVisible(true);
		layoutButtons();
	}
}

bool GraphicComboBox::eventMouseWheel(int x, int y, int zDelta) {
	if (popupShowing == true) {
		int newVisibleStart = scrollBar.getVisibleStart() -  zDelta/60;
		if (newVisibleStart < 0)
			newVisibleStart = 0;
		if (newVisibleStart > scrollBar.getLength() - scrollBar.getVisibleSize())
			newVisibleStart = scrollBar.getLength() - scrollBar.getVisibleSize();

		scrollBar.setVisibleStart(newVisibleStart);
		layoutButtons();
		mouseMoveOverButtons(x,y);
		return true;
	} else
		return false;
}

bool GraphicComboBox::mouseMoveOverButtons(int x, int y){
	bool result=false;
	if (popupShowing) {
		bool foundMouseOver=false;
		for (int i = scrollBar.getVisibleStart(); i <= scrollBar.getVisibleEnd(); ++i) {
			if (popupButtons[i]->mouseMove(x, y)) {
				foundMouseOver=true;
				result = true;
				setPreselectedItemIndex(i);
				layoutButtons();
			}
			if(!foundMouseOver){
				setPreselectedItemIndex(getSelectedItemIndex());
			}
		}
	}
	return result;
}

bool GraphicComboBox::mouseMove(int x, int y){
	if(this->getVisible() == false) {
		return false;
	}
	bool scrollbarResult=scrollBar.mouseMove(x,y);
	bool buttonResult=dropDownButton.mouseMove(x, y);
	 return scrollbarResult||buttonResult||mouseMoveOverButtons(x,y);;
}

bool GraphicComboBox::mouseClick(int x, int y) {
	if(this->getVisible() == false) {
		return false;
	}

	if(!items.empty()) {
		bool returnValue=true;
		if( dropDownButton.mouseClick(x, y)&& selectedItemIndex>=0) {
		    scrollBar.setVisibleStart(selectedItemIndex);
		    popupButtons[selectedItemIndex]->setLighted(true);
			togglePopupVisibility();
		}
		else if(scrollBar.mouseClick(x,y)){
			layoutButtons();
			returnValue=false;
		}
		else if (popupShowing){
			returnValue=false;
			for (int i = scrollBar.getVisibleStart();
							i <= scrollBar.getVisibleEnd(); ++i) {
						if( popupButtons[i]->mouseClick(x,y)){
							setSelectedItemIndex(i);
							layoutButtons();
							togglePopupVisibility();
							returnValue=true;
						}
					}
			if(returnValue==false){
				returnValue=true;
				togglePopupVisibility();
				setPreselectedItemIndex(selectedItemIndex);
			}
		}
		else{
			returnValue=false;
		}

		return returnValue;
	}
	return false;
}

bool GraphicComboBox::mouseDown(int x, int y) {
	if( scrollBar.mouseDown(x,y)){
		layoutButtons();
		return true;
	}
	return false;
}
void GraphicComboBox::mouseUp(int x, int y) {
	scrollBar.mouseUp(x,y);
}




// =====================================================
//	class GraphicMessageBox
// =====================================================

const int GraphicMessageBox::defH= 280;
const int GraphicMessageBox::defW= 350;

GraphicMessageBox::GraphicMessageBox(const std::string &containerName, const std::string &objName) :
	GraphicComponent(containerName, objName) {
	header= "";
	autoWordWrap=true;
}

GraphicMessageBox::~GraphicMessageBox(){
	removeButtons();
}

void GraphicMessageBox::removeButtons(){
	while(!buttons.empty()){
		delete buttons.back();
		buttons.pop_back();
	}
}

void GraphicMessageBox::init(const string &button1Str, const string &button2Str, int newWidth, int newHeight){
	init(button1Str, newWidth, newHeight);
	addButton(button2Str);
}

void GraphicMessageBox::init(const string &button1Str, int newWidth, int newHeight){
	init(newWidth,newHeight);
	removeButtons();
	addButton(button1Str);
}

void GraphicMessageBox::init(int newWidth, int newHeight) {
	setFont(CoreData::getInstance().getMenuFontNormal());
	setFont3D(CoreData::getInstance().getMenuFontNormal3D());
	
	h= (newHeight >= 0 ? newHeight : defH);
	w= (newWidth >= 0 ? newWidth : defW);

	const Metrics &metrics= Metrics::getInstance();

	x= (metrics.getVirtualW() - w) / 2;
	y= (metrics.getVirtualH() - h) / 2;
}

void GraphicMessageBox::addButton(const string &buttonStr, int width, int height){
	GraphicButton *newButton= new GraphicButton(containerName, instanceName + "_Button_" + buttonStr);
	newButton->init(0, 0);
	newButton->setText(buttonStr);
	if(width != -1){
		newButton->setW(width);
	}
	if(height != -1){
		newButton->setH(height);
	}
	buttons.push_back(newButton);
	alignButtons();
}

void GraphicMessageBox::alignButtons(){
	int currXPos= 0;
	int totalbuttonListLength=0;
	int buttonOffset=5;
	for(int i= 0; i < getButtonCount(); i++){
		GraphicButton *button= getButton(i);
		totalbuttonListLength+=button->getW();
	}
	totalbuttonListLength+=(getButtonCount()-1)*buttonOffset;
	currXPos=x+w/2-totalbuttonListLength/2;
	for(int i= 0; i < getButtonCount(); i++){
		GraphicButton *button= getButton(i);
		button->setY(y + 25);
		button->setX(currXPos);
		currXPos+=button->getW()+buttonOffset;
	}
}

void GraphicMessageBox::setX(int x){
	this->x= x;
	alignButtons();
}

void GraphicMessageBox::setY(int y){
	this->y= y;
	alignButtons();
}

bool GraphicMessageBox::mouseMove(int x, int y){
	if(this->getVisible() == false){
		return false;
	}
	for(int i= 0; i < getButtonCount(); i++){
		if(getButton(i)->mouseMove(x, y)){
			return true;
		}
	}
	return false;
}

bool GraphicMessageBox::mouseClick(int x, int y){
	if(this->getVisible() == false){
		return false;
	}

	for(int i= 0; i < getButtonCount(); i++){
		if(getButton(i)->mouseClick(x, y)){
			return true;
		}
	}
	return false;
}

bool GraphicMessageBox::mouseClick(int x, int y, int &clickedButton){
	if(this->getVisible() == false){
		return false;
	}

	for(int i= 0; i < getButtonCount(); i++){
		if(getButton(i)->mouseClick(x, y)){
			clickedButton=i;
			return true;
		}
	}
	return false;
}

// =====================================================
//	class GraphicLine
// =====================================================

const int GraphicLine::defH= 5;
const int GraphicLine::defW= 1000;

GraphicLine::GraphicLine(const std::string &containerName, const std::string &objName)
: GraphicComponent(containerName, objName) {
	horizontal = false;
}

void GraphicLine::init(int x, int y, int w, int h){
	GraphicComponent::init(x, y, w, h);
	horizontal=true;
}

// =====================================================
//	class GraphicCheckBox
// =====================================================

const int GraphicCheckBox::defH= 22;
const int GraphicCheckBox::defW= 22;

GraphicCheckBox::GraphicCheckBox(const std::string &containerName, const std::string &objName)
: GraphicComponent(containerName, objName) {
	value = false;
	lighted = false;
}

void GraphicCheckBox::init(int x, int y, int w, int h){
	GraphicComponent::init(x, y, w, h);
	value=true;
    lighted= false;
}

bool GraphicCheckBox::mouseMove(int x, int y){
	if(this->getVisible() == false) {
		return false;
	}

	bool b= GraphicComponent::mouseMove(x, y);
    lighted= b;
    return b;
}

bool GraphicCheckBox::mouseClick(int x, int y){
	bool result=GraphicComponent::mouseClick( x,  y);
	if(result == true) {
    	if(value) {
    		value=false;
    	}
    	else {
    		value=true;
    	}
	}
    return result;
}

// =====================================================
//	class GraphicScrollBar
// =====================================================

const int GraphicScrollBar::defThickness=20;
const int GraphicScrollBar::defLength= 200;

GraphicScrollBar::GraphicScrollBar(const std::string &containerName, const std::string &objName)
: GraphicComponent(containerName, objName) {
	lighted = false;
	activated = false;
	horizontal = false;
	elementCount = 0;
	visibleSize = 0;
	visibleStart = 0;

	// position on component for renderer
	visibleCompPosStart = 0;
	visibleCompPosEnd = 0;
}

void GraphicScrollBar::init(int x, int y, bool horizontal,int length, int thickness){
	GraphicComponent::init(x, y, horizontal?length:thickness,horizontal?thickness:length );
	this->horizontal=horizontal;
	this->elementCount=1;
	this->visibleSize=1;
	this->visibleStart=0;
	this->visibleCompPosStart=0;
	this->visibleCompPosEnd=length;
	activated = false;
	lighted = false;
}

bool GraphicScrollBar::mouseDown(int x, int y) {
	bool result=false;
	if(getVisible() && getEnabled() && getEditable())
	{
		if(activated && elementCount>0)
		{
			if( elementCount>visibleSize) {
				int pos;
				if(horizontal){
					pos=x-this->x;
				}
				else {
					// invert the clicked point ( y is from bottom to top normally )
					pos=getLength()-(y-this->y);
				}
				float partSize=(float)getLength()/(float)elementCount;
				float visiblePartSize=partSize*(float)visibleSize;
				float startPos=((float)pos)-visiblePartSize/2;

				visibleStart=startPos/partSize;
				setVisibleStart(visibleStart);
				result=true;
			}
		}
	}
	return result;
}

void GraphicScrollBar::mouseUp(int x, int y) {
	activated = false;
	lighted = false;
}

void GraphicScrollBar::setVisibleStart(int vs){
	visibleStart=vs;

	if(visibleStart>elementCount-visibleSize) {
		visibleStart=elementCount-visibleSize;
	}
	if(visibleStart<0) {
		visibleStart=0;
	}
	float partSize = 0.f;
	if(elementCount > 0) {
		partSize = (float)getLength()/(float)elementCount;
	}
	visibleCompPosStart=visibleStart*partSize;
	visibleCompPosEnd=visibleStart*partSize+visibleSize*partSize;
	if(visibleCompPosEnd>getLength()) {
		visibleCompPosEnd=getLength();
	}
	if(!horizontal) {
		// invert the display ( y is from bottom to top normally )
		visibleCompPosStart=getLength()-visibleCompPosStart;
		visibleCompPosEnd=getLength()-visibleCompPosEnd;
	}
}

void GraphicScrollBar::setElementCount(int elementCount){
	this->elementCount=elementCount;
	setVisibleStart(getVisibleStart());
}

void GraphicScrollBar::setVisibleSize(int visibleSize){
	this->visibleSize=visibleSize;
	setVisibleStart(getVisibleStart());
}


bool GraphicScrollBar::mouseClick(int x, int y){
	bool result=GraphicComponent::mouseClick( x,  y);
	if(result) {
		activated = true;
		lighted = true;
		mouseDown( x,  y);
	}
	return result;
}


bool GraphicScrollBar::mouseMove(int x, int y){
	if(this->getVisible() == false) {
		return false;
	}

	bool inScrollBar = GraphicComponent::mouseMove(x, y);
	if (activated) {
		lighted = true;
	} else {
		lighted = inScrollBar;
	}
	return inScrollBar;
}

int GraphicScrollBar::getLength() const {
	return horizontal?getW():getH();
}

//int GraphicScrollBar::getThickness() const {
//	return horizontal?getH():getW();
//}

void GraphicScrollBar::arrangeComponents(vector<GraphicComponent *> &gcs) {
	if(getElementCount()!=0 ) {
    	for(int i = getVisibleStart(); i <= getVisibleEnd(); ++i) {
    		if(horizontal){
    			gcs[i]->setX(getX()+getLength()-gcs[i]->getW()-gcs[i]->getW()*(i-getVisibleStart()));
    		}
    		else {
    			gcs[i]->setY(getY()+getLength()-gcs[i]->getH()-gcs[i]->getH()*(i-getVisibleStart()));
    		}
    	}
    }
}
// ===========================================================
// 	class PopupMenu
// ===========================================================

const int PopupMenu::defH= 240;
const int PopupMenu::defW= 350;

PopupMenu::PopupMenu(const std::string &containerName, const std::string &objName) :
		GraphicComponent(containerName, objName, false) {
	registerGraphicComponentOnlyFontCallbacks(containerName,objName);

	h= defH;
	w= defW;
}

PopupMenu::~PopupMenu() {

}

void PopupMenu::init(string menuHeader,std::vector<string> menuItems) {
	header = menuHeader;

	setFont(CoreData::getInstance().getMenuFontNormal());
	setFont3D(CoreData::getInstance().getMenuFontNormal3D());

	buttons.clear();

	const Metrics &metrics= Metrics::getInstance();

	x= (metrics.getVirtualW()-w)/2;
	y= (metrics.getVirtualH()-h)/2;

	int textHeight = GraphicButton::defH;
	int textHeightSpacing = 6;

	int maxButtonWidth = -1;
	for(unsigned int i = 0; i < menuItems.size(); ++i) {
		int currentButtonWidth = -1;
		if(font3D != NULL && Shared::Graphics::Font::forceLegacyFonts == false) {
			FontMetrics *fontMetrics= font3D->getMetrics();
			if(fontMetrics) {
				currentButtonWidth = fontMetrics->getTextWidth(menuItems[i]);
			}
		}
		else if(font) {
			FontMetrics *fontMetrics= font->getMetrics();
			if(fontMetrics) {
				currentButtonWidth = fontMetrics->getTextWidth(menuItems[i]);
			}
		}

		if(maxButtonWidth < 0 || currentButtonWidth > maxButtonWidth) {
			maxButtonWidth = currentButtonWidth + textHeightSpacing;
		}
	}

	int yStartOffset = y + h - (textHeight * 2);

	if(maxButtonWidth >= w) {
		w = maxButtonWidth + textHeightSpacing;
		x= (metrics.getVirtualW()-w)/2;
	}

	int offsetH = (yStartOffset - y);
	int maxH = (offsetH + (((int)menuItems.size() -1 ) * (textHeight + textHeightSpacing)));
	if(maxH >= h) {
		h = maxH;
		y= (metrics.getVirtualH()-h)/2;
		yStartOffset = y + h - (textHeight * 2);
	}

	for(unsigned int i = 0; i < menuItems.size(); ++i) {
		GraphicButton button(containerName, instanceName + "_Popup_Button_" + menuItems[i],false);
		button.registerGraphicComponentOnlyFontCallbacks(containerName, instanceName + "_Popup_Button_" + menuItems[i]);
		button.init(x+(w-maxButtonWidth)/2, yStartOffset - (i*(textHeight + textHeightSpacing)));
		button.setText(menuItems[i]);
		button.setW(maxButtonWidth);

		buttons.push_back(button);
	}
}

void PopupMenu::setX(int x) {
	this->x= x;

	for(unsigned int i = 0; i < buttons.size(); ++i) {
		GraphicButton &button = buttons[i];
		button.init(x+(w-GraphicButton::defW)/4, y+25 + (i*25));
	}
}

void PopupMenu::setY(int y) {
	this->y= y;

	for(unsigned int i = 0; i < buttons.size(); ++i) {
		GraphicButton &button = buttons[i];
		button.init(x+(w-GraphicButton::defW)/4, y+25 + (i*25));
	}
}

bool PopupMenu::mouseMove(int x, int y){
	if(this->getVisible() == false) {
		return false;
	}

	for(unsigned int i = 0; i < buttons.size(); ++i) {
		GraphicButton &button = buttons[i];
		if(button.mouseMove(x, y)) {
			return true;
		}
	}

	return false;
}

bool PopupMenu::mouseClick(int x, int y) {
	if(this->getVisible() == false) {
		return false;
	}

	for(unsigned int i = 0; i < buttons.size(); ++i) {
		GraphicButton &button = buttons[i];
		if(button.mouseClick(x, y)) {
			return true;
		}
	}
	return false;
}

std::pair<int,string> PopupMenu::mouseClickedMenuItem(int x, int y) {
	std::pair<int,string> result;
	for(unsigned int i = 0; i < buttons.size(); ++i) {
		GraphicButton &button = buttons[i];
		if(button.mouseClick(x, y)) {
			result.first = i;
			result.second = buttons[i].getText();
			break;
		}
	}

	return result;
}

}}//end namespace
