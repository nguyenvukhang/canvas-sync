function Component()
{
    component.loaded.connect(this, Component.prototype.loaded);
}

Component.prototype.createOperations = function()
{
    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut",
                "@TargetDir@/Canvas Sync.exe",
                "@StartMenuDir@/Canvas Sync.lnk",
                "iconPath=@TargetDir@/appicon.ico",
                "workingDirectory=@TargetDir@");
    }
}


Component.prototype.isDefault = function() { return true; }
