sap.ui.define([
	'jquery.sap.global',
	'sap/ui/core/Fragment',
	'sap/m/MessageToast',
	'./Formatter',
	'sap/ui/core/mvc/Controller',
	'sap/ui/model/json/JSONModel',
	'sap/m/Popover',
	'sap/m/UploadCollectionParameter',
	'sap/m/Button',
	"sap/ui/Device",
], function (jQuery, Fragment, MessageToast, Formatter, Controller, JSONModel, Popover, Button, Device) {
	"use strict";

	var oModelData           = new sap.ui.model.json.JSONModel();
	var ip_address = location.host;
	
	var ws = new WebSocket(`ws://${ip_address}/ws`);

	var CController = Controller.extend("view.App", {		
		
		onInit: function() {
			var oView = this.getView();

			// Determine visible tiles in carousel
			var visibleTiles = 1;
			if (Device.system.desktop) {
				visibleTiles = 8;
			} else if (Device.system.tablet) {
				visibleTiles = 2;
			}
			var oSettingsModel = new JSONModel({ pagesCount: iPagesCount });
			this.getView().setModel(oSettingsModel, "settings");

			// Dynamisches Menü						
			var MenuModel = new JSONModel("./static/menu.json");
			oView.setModel(MenuModel);			


			 
			ws.onopen = function() {                  
				// Web Socket is connected, send data using send()			
			   //	 alert("WS open im controller");
				// ws.send("Hallo from Client");
			 };	 
				
			ws.onmessage = function (evt)  { 
				//alert("WS open2 im controller" + evt.data);				
				oModelData.setData(jQuery.parseJSON(evt.data));	
				oView.setModel(oModelData,"dataBuffer");

			 }; 	
			 
		},				

		onItemSelect: function(oEvent) {
			var item = oEvent.getParameter('item');
			var viewId = this.getView().getId();
			sap.ui.getCore().byId(viewId + "--pageContainer").to(viewId + "--" + item.getKey());
		},

		reboot: function (oEvent) {
			alert("LED toggled");
			ws.send("reboot");		
		},
	

		onSideNavButtonPress: function() {
			var viewId = this.getView().getId();
			var toolPage = sap.ui.getCore().byId(viewId + "--toolPage");
			var sideExpanded = toolPage.getSideExpanded();
			this._setToggleButtonTooltip(sideExpanded);
			toolPage.setSideExpanded(!toolPage.getSideExpanded());
		},


		_setToggleButtonTooltip: function(bLarge) {
			var toggleButton = this.byId('sideNavigationToggleButton');
			if (bLarge) {
				toggleButton.setTooltip('Large Size Navigation');
			} else {
				toggleButton.setTooltip('Small Size Navigation');
			}
		}

	});

	return CController;

});