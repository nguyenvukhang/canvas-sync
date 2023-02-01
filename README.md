## Canvas Sync

A cross-platform [compilable\*](#cross-platformness) tool that keeps local folders up-to-date
with online folders on Canvas.

- [Install](#install)
- [Usage](#usage)

### Install

To install Canvas Sync, head over to the [latest release][latest] and
download a file that is specific to your operating system.

For **macOS** users, after dragging CanvasSync.app into your
`/Applications` folder, you have to open Terminal and run

```
xattr -c /Applications/CanvasSync.app
```

in order to be able to open the app.

### Usage

Upon first launch, you have to supply an Access Token to Canvas. This
can be found at your [canvas profile
settings](https://canvas.nus.edu.sg/profile/settings) and looking
around for the 'token' keyword. Generate a fresh one, copy and paste
it into Canvas Sync. Canvas Sync will save it for you and reuse it on
the next launch.

### Cross platformness

Currently compiled for MacOS and Windows, but since it's written with
the Qt framework, it can be easily adapted to compile for Linux
distributions. PRs are welcome.

A tool for students to keep local folders in sync with remote folders
on Canvas.

[latest]: https://github.com/nguyenvukhang/canvas-sync/releases/latest
