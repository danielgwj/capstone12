# Better Graphics For a Robotics Grasping GUI
### Team 12: Shady Robots ###
=============

[Link to OSUrobotics's Grasp Rendering Modification repo](https://github.com/OSUrobotics/Grasp-Rendering-Modification)

[OneNote Notebook (view-only)](https://1drv.ms/u/s!Av4EOp0PXBcCjGQ96cSX6slmyb54)

[Virtual Machine Image Download](https://drive.google.com/open?id=0BzM4_ayQHUjNUEUyVmlIanl4bVk)

Login: mcqueen

Password: openrave

=============

##Working with the submodule

Cloning your repo and initializing the submodule:
```
git clone https://github.com/danielgwj/capstone12.git
cd capstone12
cd Grasp-Rendering-Modification
git submodule init
git submodule update
```
=============

Working with branches:
```
git checkout <branchname>  ### TO CHECKOUT AN EXISTING BRANCH
```
OR
```
git checkout -b <branchname> ### TO CREATE A NEW BRANCH
```

=============

To merge your branch changes to the submodule's repo:

1. Commit and push the code changes to your branch.

2. Navigate to submodule's repo on a browser.

3. Do a "Compare & pull request".

4. Create a pull request.

5. Merge pull request (either yourself or another team member).

=============

##Information on QT Rendering

###Important files to note in libsoqt source files
- Coin3D/src/Inventor/QT/SoQtGLWidget.cpp
- Coin3D/src/Inventor/QT/SoQT.cpp

[Link to download libsoqt source files](http://packages.ubuntu.com/trusty/libsoqt-dev-common)
