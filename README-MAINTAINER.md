[![license](https://img.shields.io/github/license/micro-os-plus/semihosting-xpack)](https://github.com/micro-os-plus/semihosting-xpack/blob/xpack/LICENSE)
[![CI on Push](https://github.com/micro-os-plus/semihosting-xpack/actions/workflows/CI.yml/badge.svg)](https://github.com/micro-os-plus/semihosting-xpack/actions/workflows/CI.yml)
[![GitHub issues](https://img.shields.io/github/issues/micro-os-plus/semihosting-xpack.svg)](https://github.com/micro-os-plus/semihosting-xpack/issues/)
[![GitHub pulls](https://img.shields.io/github/issues-pr/micro-os-plus/semihosting-xpack.svg)](https://github.com/micro-os-plus/semihosting-xpack/pulls)

# Maintainer info

## Project repository

The project is hosted on GitHub:

- <https://github.com/micro-os-plus/semihosting-xpack.git>

To clone the stable branch (`xpack`), run the following commands in a
terminal (on Windows use the _Git Bash_ console):

```sh
rm -rf ~/Work/semihosting-xpack.git && \
mkdir -p ~/Work && \
git clone \
  https://github.com/micro-os-plus/semihosting-xpack.git \
  ~/Work/semihosting-xpack.git
```

For development purposes, clone the `xpack-develop` branch:

```sh
rm -rf ~/Work/semihosting-xpack.git && \
mkdir -p ~/Work && \
git clone \
  --branch xpack-develop \
  https://github.com/micro-os-plus/semihosting-xpack.git \
  ~/Work/semihosting-xpack.git
```

## Prerequisites

A recent [xpm](https://xpack.github.io/xpm/), which is a portable
[Node.js](https://nodejs.org/) command line application.

## Code formatting

Code formatting is done using `clang-format --style=file`, either manually
from a script, or automatically from Visual Studio Code, or the Eclipse
CppStyle plug-in.

Always reformat the source files that were changed.

## How to make new releases

### Release schedule

There are no fixed releases.

### Check Git

In the `micro-os-plus/semihosting-xpack` Git repo:

- switch to the `xpack-develop` branch
- if needed, merge the `xpack` branch

No need to add a tag here, it'll be added when the release is created.

### Increase the version

Determine the upstream version (like `8.0.0`) and eventually update the
`package.json` file; the format is `8.0.0-pre`.

### Fix possible open issues

Check GitHub issues and pull requests:

- <https://github.com/micro-os-plus/semihosting-xpack/issues/>

and fix them; assign them to a milestone (like `8.0.0`).

### Update `README-MAINTAINER.md`

Update the `README-MAINTAINER.md` file to reflect the changes
related to the new version.

### Update `CHANGELOG.md`

- open the `CHANGELOG.md` file
- check if all previous fixed issues are in
- add a new entry like _* v8.0.0_
- commit with a message like _prepare v8.0.0_

### Push changes

- reformat the source files that were changed
- commit and push

### Manual tests

To run the tests manually on the local machine:

```sh
cd ~Work/semihosting-xpack.git

xpm run install-all
xpm run test-all
```

## Publish on the npmjs.com server

- select the `xpack-develop` branch
- commit all changes
- `npm pack` and check the content of the archive, which should list
  only `package.json`, `README.md`, `LICENSE`, `CHANGELOG.md`,
  the sources and CMake/meson files;
  possibly adjust `.npmignore`
- `npm version patch`, `npm version minor`, `npm version major`
- push the `xpack-develop` branch to GitHub
- the `postversion` npm script should also update tags via `git push origin --tags`
- wait for the CI job to complete
  (<https://github.com/micro-os-plus/semihosting-xpack/actions/workflows/CI.yml>)

### Test on all platforms

In addition, it is possible to manually trigger a **test-all** job, that
runs all available builds, on all supported platforms, including Linux Arm
and macOS Apple Silicon.

For this:

- start the `~/actions-runners/micro-os-plus/run.sh &` runner on `xbbma` and `xbbla`
- ensure that the `xpack-develop` branch is pushed
- run the `trigger-workflow-test-all` action
- wait for the **test-all** job to complete
  (<https://github.com/micro-os-plus/semihosting-xpack/actions/workflows/test-all.yml>)

### Publish

- `npm publish --tag next` (use `npm publish --access public` when
  publishing for the first time)

The version is visible at:

- <https://www.npmjs.com/package/@micro-os-plus/semihosting?activeTab=versions>

## Update the repo

When the package is considered stable:

- with a Git client (VS Code is fine)
- merge `xpack-develop` into `xpack`
- push to GitHub
- select `xpack-develop`

## Tag the npm package as `latest`

When the release is considered stable, promote it as `latest`:

- `npm dist-tag ls @micro-os-plus/semihosting`
- `npm dist-tag add @micro-os-plus/semihosting@8.0.0 latest`
- `npm dist-tag ls @micro-os-plus/semihosting`
