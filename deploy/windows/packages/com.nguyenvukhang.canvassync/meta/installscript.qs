function Component()
{
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut",
                "@TargetDir@/Canvas Sync.exe",
                "@StartMenuDir@/Canvas Sync.lnk",
                "iconPath=@TargetDir@/appicon.ico",
                "workingDirectory=@TargetDir@");
    }
}
