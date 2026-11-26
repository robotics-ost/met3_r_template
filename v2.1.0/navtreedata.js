/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "Mobiler Ausbildungsroboter", "index.html", [
    [ "Mobiler Ausbildungsroboter ESP32", "index.html", "index" ],
    [ "System Architecture", "nav_architecture.html", null ],
    [ "Guides", "nav_guides.html", null ],
    [ "Exercises", "nav_exercises.html", "nav_exercises" ],
    [ "Troubleshooting", "md_docs_2TROUBLESHOOTING.html", [
      [ "Build", "md_docs_2TROUBLESHOOTING.html#build", [
        [ "<span class=\"tt\">include/wifi_secrets.h is missing</span>", "md_docs_2TROUBLESHOOTING.html#includewifi_secretsh-is-missing", null ],
        [ "The build fails with strange errors right from the start", "md_docs_2TROUBLESHOOTING.html#the-build-fails-with-strange-errors-right-from-the-start", null ],
        [ "The first build takes very long", "md_docs_2TROUBLESHOOTING.html#the-first-build-takes-very-long", null ],
        [ "<span class=\"tt\">fatal error: motor_model.h: No such file or directory</span>", "md_docs_2TROUBLESHOOTING.html#fatal-error-motor_modelh-no-such-file-or-directory", null ],
        [ "<span class=\"tt\">undefined reference to 'motor_model_init'</span>", "md_docs_2TROUBLESHOOTING.html#undefined-reference-to-motor_model_init", null ],
        [ "<span class=\"tt\">implicit declaration of function '...'</span>", "md_docs_2TROUBLESHOOTING.html#implicit-declaration-of-function-", null ],
        [ "VS Code shows red squiggly lines, but the build works", "md_docs_2TROUBLESHOOTING.html#vs-code-shows-red-squiggly-lines-but-the-build-works", null ]
      ] ],
      [ "Upload", "md_docs_2TROUBLESHOOTING.html#upload", [
        [ "The robot is not found / <span class=\"tt\">Could not open port</span> / no COM port", "md_docs_2TROUBLESHOOTING.html#the-robot-is-not-found--could-not-open-port--no-com-port", null ],
        [ "The upload hangs at <span class=\"tt\">Connecting........_____</span>", "md_docs_2TROUBLESHOOTING.html#the-upload-hangs-at-connecting_____", null ]
      ] ],
      [ "Serial Monitor and crashes", "md_docs_2TROUBLESHOOTING.html#serial-monitor-and-crashes", [
        [ "The Serial Monitor shows unreadable characters", "md_docs_2TROUBLESHOOTING.html#the-serial-monitor-shows-unreadable-characters", null ],
        [ "The robot restarts again and again", "md_docs_2TROUBLESHOOTING.html#the-robot-restarts-again-and-again", null ]
      ] ],
      [ "Motors", "md_docs_2TROUBLESHOOTING.html#motors", [
        [ "The motors don't move", "md_docs_2TROUBLESHOOTING.html#the-motors-dont-move", null ],
        [ "A wheel turns in the wrong direction", "md_docs_2TROUBLESHOOTING.html#a-wheel-turns-in-the-wrong-direction", null ]
      ] ],
      [ "WiFi and web GUI", "md_docs_2TROUBLESHOOTING.html#wifi-and-web-gui", [
        [ "The robot doesn't connect to the WiFi", "md_docs_2TROUBLESHOOTING.html#the-robot-doesnt-connect-to-the-wifi", null ],
        [ "The web GUI doesn't start: <span class=\"tt\">Could not resolve host</span>", "md_docs_2TROUBLESHOOTING.html#the-web-gui-doesnt-start-could-not-resolve-host", null ],
        [ "The web GUI starts, but shows no data", "md_docs_2TROUBLESHOOTING.html#the-web-gui-starts-but-shows-no-data", null ],
        [ "The robot doesn't respond anymore after <em>Do System Off</em>", "md_docs_2TROUBLESHOOTING.html#the-robot-doesnt-respond-anymore-after-do-system-off", null ],
        [ "The plots stopped working after I changed the telemetry", "md_docs_2TROUBLESHOOTING.html#the-plots-stopped-working-after-i-changed-the-telemetry", null ],
        [ "Windows: <span class=\"tt\">running scripts is disabled on this system</span>", "md_docs_2TROUBLESHOOTING.html#windows-running-scripts-is-disabled-on-this-system", null ],
        [ "<span class=\"tt\">python</span> or <span class=\"tt\">pip</span> is not found", "md_docs_2TROUBLESHOOTING.html#python-or-pip-is-not-found", null ]
      ] ],
      [ "Git and GitHub", "md_docs_2TROUBLESHOOTING.html#git-and-github", null ],
      [ "Still stuck?", "md_docs_2TROUBLESHOOTING.html#still-stuck", null ]
    ] ],
    [ "C for Python Programmers", "md_docs_2C__FOR__PYTHON__PROGRAMMERS.html", [
      [ "Common mistakes", "md_docs_2C__FOR__PYTHON__PROGRAMMERS.html#common-mistakes", null ],
      [ "The basics", "md_docs_2C__FOR__PYTHON__PROGRAMMERS.html#the-basics", [
        [ "<span class=\"tt\">if</span>, <span class=\"tt\">for</span> and <span class=\"tt\">while</span>", "md_docs_2C__FOR__PYTHON__PROGRAMMERS.html#if-for-and-while", null ]
      ] ],
      [ "Types", "md_docs_2C__FOR__PYTHON__PROGRAMMERS.html#types", null ],
      [ "Arrays", "md_docs_2C__FOR__PYTHON__PROGRAMMERS.html#arrays", null ],
      [ "Pointers and addresses", "md_docs_2C__FOR__PYTHON__PROGRAMMERS.html#pointers-and-addresses", null ],
      [ "Functions return results through pointers", "md_docs_2C__FOR__PYTHON__PROGRAMMERS.html#functions-return-results-through-pointers", null ],
      [ "Structs", "md_docs_2C__FOR__PYTHON__PROGRAMMERS.html#structs", null ],
      [ "Header and source files", "md_docs_2C__FOR__PYTHON__PROGRAMMERS.html#header-and-source-files", null ],
      [ "<span class=\"tt\">static</span>", "md_docs_2C__FOR__PYTHON__PROGRAMMERS.html#static", null ],
      [ "Constants with <span class=\"tt\">#define</span>", "md_docs_2C__FOR__PYTHON__PROGRAMMERS.html#constants-with-define", null ],
      [ "Error handling", "md_docs_2C__FOR__PYTHON__PROGRAMMERS.html#error-handling", null ],
      [ "Printing values", "md_docs_2C__FOR__PYTHON__PROGRAMMERS.html#printing-values", null ]
    ] ],
    [ "Control System", "md_docs_2CONTROL__SYSTEM.html", [
      [ "The concept", "md_docs_2CONTROL__SYSTEM.html#the-concept", [
        [ "Sharing data with other tasks", "md_docs_2CONTROL__SYSTEM.html#sharing-data-with-other-tasks", null ]
      ] ],
      [ "In the robot software", "md_docs_2CONTROL__SYSTEM.html#in-the-robot-software", [
        [ "The control task", "md_docs_2CONTROL__SYSTEM.html#the-control-task", null ],
        [ "Modules", "md_docs_2CONTROL__SYSTEM.html#modules", null ],
        [ "Example: the encoder driver", "md_docs_2CONTROL__SYSTEM.html#example-the-encoder-driver", null ],
        [ "The control system and the safety level", "md_docs_2CONTROL__SYSTEM.html#the-control-system-and-the-safety-level", null ]
      ] ]
    ] ],
    [ "Getting Started: Software Installation", "md_docs_2GETTING__STARTED.html", [
      [ "Overview", "md_docs_2GETTING__STARTED.html#overview", null ],
      [ "Step 1: GitHub account", "md_docs_2GETTING__STARTED.html#step-1-github-account", null ],
      [ "Step 2: Git", "md_docs_2GETTING__STARTED.html#step-2-git", null ],
      [ "Step 3: VS Code", "md_docs_2GETTING__STARTED.html#step-3-vs-code", null ],
      [ "Step 4: PlatformIO extension", "md_docs_2GETTING__STARTED.html#step-4-platformio-extension", null ],
      [ "Step 5: USB driver (CP210x)", "md_docs_2GETTING__STARTED.html#step-5-usb-driver-cp210x", [
        [ "Windows", "md_docs_2GETTING__STARTED.html#windows", null ],
        [ "macOS", "md_docs_2GETTING__STARTED.html#macos", null ],
        [ "Linux", "md_docs_2GETTING__STARTED.html#linux", null ]
      ] ],
      [ "Step 6: Python 3", "md_docs_2GETTING__STARTED.html#step-6-python-3", null ],
      [ "Checklist", "md_docs_2GETTING__STARTED.html#checklist", null ]
    ] ],
    [ "Git and GitHub Guide", "md_docs_2GIT__GUIDE.html", [
      [ "The most important terms", "md_docs_2GIT__GUIDE.html#the-most-important-terms", null ],
      [ "Getting your repository", "md_docs_2GIT__GUIDE.html#getting-your-repository", null ],
      [ "Everyday workflow", "md_docs_2GIT__GUIDE.html#everyday-workflow", [
        [ "1. Check your changes", "md_docs_2GIT__GUIDE.html#autotoc_md1-check-your-changes", null ],
        [ "2. Stage", "md_docs_2GIT__GUIDE.html#autotoc_md2-stage", null ],
        [ "3. Commit", "md_docs_2GIT__GUIDE.html#autotoc_md3-commit", null ],
        [ "4. Push", "md_docs_2GIT__GUIDE.html#autotoc_md4-push", null ],
        [ "Terminal commands", "md_docs_2GIT__GUIDE.html#terminal-commands", null ]
      ] ],
      [ "Marking a finished exercise with a tag", "md_docs_2GIT__GUIDE.html#marking-a-finished-exercise-with-a-tag", [
        [ "Looking at an earlier solution", "md_docs_2GIT__GUIDE.html#looking-at-an-earlier-solution", null ]
      ] ],
      [ "Optional: trying something out on a branch", "md_docs_2GIT__GUIDE.html#optional-trying-something-out-on-a-branch", null ],
      [ "Working on more than one computer", "md_docs_2GIT__GUIDE.html#working-on-more-than-one-computer", null ],
      [ "Common problems", "md_docs_2GIT__GUIDE.html#common-problems", [
        [ "\"Please tell me who you are\" / \"Make sure you configure your user.name and user.email\"", "md_docs_2GIT__GUIDE.html#please-tell-me-who-you-are--make-sure-you-configure-your-username-and-useremail", null ],
        [ "Push rejected: \"Updates were rejected because the remote contains work that you do not have locally\"", "md_docs_2GIT__GUIDE.html#push-rejected-updates-were-rejected-because-the-remote-contains-work-that-you-do-not-have-locally", null ],
        [ "Merge conflict", "md_docs_2GIT__GUIDE.html#merge-conflict", null ],
        [ "I want to undo my changes to a file", "md_docs_2GIT__GUIDE.html#i-want-to-undo-my-changes-to-a-file", null ],
        [ "I want to undo my last commit (not pushed yet)", "md_docs_2GIT__GUIDE.html#i-want-to-undo-my-last-commit-not-pushed-yet", null ],
        [ "I tagged the wrong commit", "md_docs_2GIT__GUIDE.html#i-tagged-the-wrong-commit", null ],
        [ "A pull request from \"dependabot\" appeared on GitHub", "md_docs_2GIT__GUIDE.html#a-pull-request-from-dependabot-appeared-on-github", null ]
      ] ]
    ] ],
    [ "Hardware Abstraction Layer (HAL)", "md_docs_2HAL.html", [
      [ "The concept", "md_docs_2HAL.html#the-concept-1", null ],
      [ "In the robot software", "md_docs_2HAL.html#in-the-robot-software-1", [
        [ "Motor driver", "md_docs_2HAL.html#motor-driver", null ],
        [ "Encoder driver", "md_docs_2HAL.html#encoder-driver", null ],
        [ "Initialization", "md_docs_2HAL.html#initialization", null ]
      ] ]
    ] ],
    [ "Robot Software", "md_docs_2ROBOT__SOFTWARE.html", [
      [ "Components and folders", "md_docs_2ROBOT__SOFTWARE.html#components-and-folders", null ],
      [ "Tasks, cores and periods", "md_docs_2ROBOT__SOFTWARE.html#tasks-cores-and-periods", null ]
    ] ],
    [ "Safety System", "md_docs_2SAFETY__SYSTEM.html", [
      [ "The concept", "md_docs_2SAFETY__SYSTEM.html#the-concept-2", null ],
      [ "In the robot software", "md_docs_2SAFETY__SYSTEM.html#in-the-robot-software-2", [
        [ "Safety levels and events", "md_docs_2SAFETY__SYSTEM.html#safety-levels-and-events", null ],
        [ "The events and their codes", "md_docs_2SAFETY__SYSTEM.html#the-events-and-their-codes", null ],
        [ "How other components use the safety system", "md_docs_2SAFETY__SYSTEM.html#how-other-components-use-the-safety-system", null ],
        [ "What the safety level does not do", "md_docs_2SAFETY__SYSTEM.html#what-the-safety-level-does-not-do", null ]
      ] ]
    ] ],
    [ "Sequencer", "md_docs_2SEQUENCER.html", [
      [ "The concept", "md_docs_2SEQUENCER.html#the-concept-3", [
        [ "How a step is executed", "md_docs_2SEQUENCER.html#how-a-step-is-executed", null ]
      ] ],
      [ "In the robot software", "md_docs_2SEQUENCER.html#in-the-robot-software-3", [
        [ "The queue", "md_docs_2SEQUENCER.html#the-queue", null ],
        [ "Step types", "md_docs_2SEQUENCER.html#step-types", null ],
        [ "The code", "md_docs_2SEQUENCER.html#the-code", null ],
        [ "The MOVE_TO step", "md_docs_2SEQUENCER.html#the-move_to-step", null ],
        [ "Steps and the safety level", "md_docs_2SEQUENCER.html#steps-and-the-safety-level", null ],
        [ "Adding a new step type", "md_docs_2SEQUENCER.html#adding-a-new-step-type", null ]
      ] ]
    ] ],
    [ "Architecture Concept", "md_docs_2SYSTEM__ARCHITECTURE.html", [
      [ "The components", "md_docs_2SYSTEM__ARCHITECTURE.html#the-components", null ],
      [ "How they work together", "md_docs_2SYSTEM__ARCHITECTURE.html#how-they-work-together", [
        [ "Rules", "md_docs_2SYSTEM__ARCHITECTURE.html#rules", null ]
      ] ],
      [ "Why split the software?", "md_docs_2SYSTEM__ARCHITECTURE.html#why-split-the-software", [
        [ "Different timing", "md_docs_2SYSTEM__ARCHITECTURE.html#different-timing", null ],
        [ "Independence and safety", "md_docs_2SYSTEM__ARCHITECTURE.html#independence-and-safety", null ],
        [ "Shared data", "md_docs_2SYSTEM__ARCHITECTURE.html#shared-data", null ]
      ] ],
      [ "Next", "md_docs_2SYSTEM__ARCHITECTURE.html#next", null ]
    ] ],
    [ "UDP Interface", "md_docs_2UDP__INTERFACE.html", [
      [ "The concept", "md_docs_2UDP__INTERFACE.html#the-concept-4", null ],
      [ "In the robot software", "md_docs_2UDP__INTERFACE.html#in-the-robot-software-4", [
        [ "Connection", "md_docs_2UDP__INTERFACE.html#connection", null ],
        [ "Telemetry", "md_docs_2UDP__INTERFACE.html#telemetry", null ],
        [ "Commands", "md_docs_2UDP__INTERFACE.html#commands", null ],
        [ "Adapting the interface", "md_docs_2UDP__INTERFACE.html#adapting-the-interface", null ]
      ] ]
    ] ],
    [ "Web GUI", "md_docs_2WEB__GUI.html", [
      [ "Set up (once per computer)", "md_docs_2WEB__GUI.html#set-up-once-per-computer", null ],
      [ "Start the GUI (every time)", "md_docs_2WEB__GUI.html#start-the-gui-every-time", null ],
      [ "Using the GUI", "md_docs_2WEB__GUI.html#using-the-gui", null ],
      [ "Something doesn't work?", "md_docs_2WEB__GUI.html#something-doesnt-work", null ],
      [ "Changing the GUI", "md_docs_2WEB__GUI.html#changing-the-gui", null ]
    ] ],
    [ "Working with the Code", "md_docs_2WORKING__WITH__THE__CODE.html", [
      [ "How the program runs", "md_docs_2WORKING__WITH__THE__CODE.html#how-the-program-runs", null ],
      [ "Adding a module from the starter code", "md_docs_2WORKING__WITH__THE__CODE.html#adding-a-module-from-the-starter-code", [
        [ "Step 1: Copy the files", "md_docs_2WORKING__WITH__THE__CODE.html#step-1-copy-the-files", null ],
        [ "Step 2: Fill in the TO DOs", "md_docs_2WORKING__WITH__THE__CODE.html#step-2-fill-in-the-to-dos", null ],
        [ "Step 3: Add the configuration", "md_docs_2WORKING__WITH__THE__CODE.html#step-3-add-the-configuration", null ],
        [ "Step 4: Initialize the module", "md_docs_2WORKING__WITH__THE__CODE.html#step-4-initialize-the-module", null ],
        [ "Step 5: Use the module in the control loop", "md_docs_2WORKING__WITH__THE__CODE.html#step-5-use-the-module-in-the-control-loop", null ]
      ] ],
      [ "Seeing what your code does", "md_docs_2WORKING__WITH__THE__CODE.html#seeing-what-your-code-does", [
        [ "Messages in the Serial Monitor", "md_docs_2WORKING__WITH__THE__CODE.html#messages-in-the-serial-monitor", null ],
        [ "Plots in the web GUI", "md_docs_2WORKING__WITH__THE__CODE.html#plots-in-the-web-gui", null ]
      ] ],
      [ "Vectors and arrays", "md_docs_2WORKING__WITH__THE__CODE.html#vectors-and-arrays", null ]
    ] ],
    [ "Todo List", "todo.html", null ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ]
      ] ]
    ] ],
    [ "Data Structures", "annotated.html", [
      [ "Data Structures", "annotated.html", "annotated_dup" ],
      [ "Data Structure Index", "classes.html", null ],
      [ "Data Fields", "functions.html", [
        [ "All", "functions.html", null ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "Globals", "globals.html", [
        [ "All", "globals.html", "globals_dup" ],
        [ "Functions", "globals_func.html", null ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"index.html#step-8-connect-to-the-robot-from-your-pc",
"motor__model_8c.html#a4e0bd333ddc7adb461df74410ff05b3e",
"structpi__controller__config__t.html#a1f7add0f84d583bc2eceb290d6c4cc06"
];

const SYNCONMSG = 'click to disable panel synchronization';
const SYNCOFFMSG = 'click to enable panel synchronization';
const LISTOFALLMEMBERS = 'List of all members';