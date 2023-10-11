# CPak
CPak is a CLI tool for the modern developer to compile and package manage their C/C++ projects with a configuration as code architecture. With a configuration as code architecture, end-user first design, and key emphasis on simplification and usability, CPak aims to redefine what C/C++ build tools are and how developers interact with them. Ultimately making the landscape smooth all around for new-comers and well established professionals.

## Getting started
As of writing this, CPak does not currently have a release package built yet. This will be coming very soon. Here's what you'll need:
* C++20 compatible compiler
* CMake 3.16 or greater

Here are the steps to compile and install:
* `git clone --recursive-submodules https://github.com/SoraKatadzuma/cpak`
* Build CPak:
    * Note: you'll need administrator or root access to install. If you do not have it, exclude the `-i` option from the build command. Feel free to copy the executable to a suitable location and add it to your PATH.
    * `./scripts/build.sh -cri` (Linux)
    * `./scripts/build.bat -c -r -i` (Windows)

Using CPak is as easy as adding a CPakFile to the root of your project. The most simplistic CPakFile contains the project description and at least 1 build target. Below is an example of that:
```yaml
project:
  name: SampleProject
  gpid: SampleCompany
  semv: 1.0.0-alpha+dev

targets:
- name: myexe
  type: executable
  search:
    include:
    - ./include
  sources:
  - main.cpp
```
With the above configuration defined, you can easily run `cpak build` and CPak will proceed to compile your code into the targets that you specified. CPak will log to the console what is being compiled, what targets are built, and where they will be placed. If you need dependencies for your project, you'll want to make sure you add a CPakFile to those dependencies if they do not already support one. You can add a section to your CPakFile for defining those dependencies, here is an example of that:
```yaml
dependencies:
- SoraKatadzuma/cpak@1.0.0
- cpakid: SampleCompany/SampleProject@1.0.0-alpha+dev
  remote:
    address: https://somerepository.com/
    username: MyUsername
    email: myusername@email.com
```
Above you can see the different styles by which you can define your dependencies. They both use the CPakID format which is defined by `organization/project@version`. It will go to the configured repository (`https://github.com/` by default) at the organization name (may also be a username) and the project name, and search for a branch or tag with the version string. So for example, the first dependency will go to `https://github.com/SoraKatadzuma/cpak` repository and look for a branch or tag labelled `1.0.0`. If it exists, it will clone it to a local repository location (`~/.cpak/organization/project@version` by default) and proceed to load that dependency's configuration. Before building your project, it will build the dependencies first, just as you may expect. Just as a note, the second dependency shows an example of how to specify a custom repository location with minimal login information if needed. In the second dependency example, it only requires the address, the `username` and `email` are optional. This may be reworked in the future so use it sparingly.

I'll be working on a formal website to explain all the internals, configurations, and options so be on the lookout for that in the future. Feel free to explore the code base to see what other things can be done with the tool, and also make sure to run `cpak -h` to see all the available commands.

## Motivation
When you look at tools like CMake, Conan, Bazel, Gradle, VCBuild, you see a vast array of different techniques. Some build upon other tools (Conan upon CMake upon make), and some try to be their own thing (literally the rest of them). Something I personally feel they miss the mark on is simplification and portability. They hit either one or the other and never both, though I do think CMake/Conan are the closest to reaching both. Understandably some of them are complex because they do complex things. Tasks that the companies that built them need them to do, but it can be hard for individuals to navigate even with the extensive documentation that is available. In my time developing I have used or tried all of the aforementioned build tools and have never been satisfied with the experience.

For that reason, and others that I could discuss at later dates, I decided to create my own. I get the flexibility to make it work the way I want and I get the ability to simplify the usage of the tool in ways that the other tools may not be able to. It's a prime position to be able to redefine what C/C++ build tools are and how they function. Not to mention, having a built in dependency manager is a handy to have instead of needing to rely on an external tool.