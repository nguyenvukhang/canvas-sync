Component.prototype.createOperations = function()
{
    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", "@TargetDir@/Canvas Sync.exe", "@StartMenuDir@/Canvas Sync.lnk",
            "workingDirectory=@TargetDir@", "iconPath=%SystemRoot%/system32/SHELL32.dll",
            "iconId=2", "description=Open Canvas Sync");
    }
}
