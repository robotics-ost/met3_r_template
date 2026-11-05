# Git and GitHub Guide

**Git** records the changes to your code, so you can always go back to an earlier state. **GitHub** stores a copy of your repository online, as a backup and so your lecturers can see your work.

You only need a handful of actions for the exercises. This guide shows them in VS Code, with the equivalent terminal commands for reference.

> [!IMPORTANT]
> **Three rules that prevent almost all problems:**
> 1. **Commit and push at the end of every lab session.** Only pushed work is safe on GitHub.
> 2. **Pull before you start working** if you use more than one computer.
> 3. **Never put passwords into files that are committed.** The WiFi password belongs only into `include/wifi_secrets.h`, which Git ignores.

> [!NOTE]
> Keyboard shortcuts are given for Windows and Linux. On macOS, use `Cmd` instead of `Ctrl`.

## The most important terms

| Term | Meaning |
|---|---|
| **Repository** (repo) | Your project folder including its complete history |
| **Clone** | Download a repository from GitHub to your computer (once per computer) |
| **Stage** | Select the changes that go into the next commit |
| **Commit** | Save a snapshot of the staged changes **locally**, with a short message |
| **Push** | Upload your commits to GitHub |
| **Pull** | Download new commits from GitHub to your computer |
| **Sync** | Pull and push in one step (VS Code button **Sync Changes**) |
| **Tag** | A fixed name for one commit, e.g. `exercise-02` for your finished solution |
| **Branch** | A separate line of development (optional, see below) |

## Getting your repository

Creating your repository and cloning it is described in the [Quick Start](../README.md#quick-start), Steps 2 and 3. You only do this once.

## Everyday workflow

All Git actions are in the **Source Control** view. Open it with the Source Control icon in the left side bar. A number on the icon shows how many files you have changed.

### 1. Check your changes

The Source Control view lists all changed files under **Changes**. Click a file to see what has changed: removed lines are red, added lines are green.

### 2. Stage

Staging means choosing which changes go into the next commit. Hover over a file under **Changes** and click the **+** icon (*Stage Changes*). The file moves to **Staged Changes**.

Stage only the files that belong to the change you want to commit. Leave out files you changed by accident or only for a quick test.

> [!TIP]
> To undo staging, hover over the file under **Staged Changes** and click the **minus** icon (*Unstage Changes*).

### 3. Commit

1. Enter a short message in the text field above the **Commit** button, describing *what* you changed.
2. Click **Commit**. Only the staged changes are committed.

> [!NOTE]
> If VS Code asks *"There are no staged changes to commit. Would you like to stage all your changes and commit them directly?"*, click **Cancel** and stage the files you want first.

Good commit messages are short and specific:

| Good | Bad |
|---|---|
| `Read encoder angles in control loop` | `changes` |
| `Add PD controller and tune gains` | `asdf` |
| `Fix wrong sign of motor B` | `final version 2` |

Commit as often as you like, e.g. every time something works. Small commits make it easier to find out what broke your code.

### 4. Push

After committing, the button changes to **Sync Changes**. Click it to push your commits to GitHub (and pull new ones at the same time).

The first time, VS Code asks you to sign in to GitHub. Allow it and sign in in the browser window that opens.

### Terminal commands

The same workflow in the terminal (`Ctrl+J` opens the terminal in VS Code):

```bash
git status                       # show changed and staged files
git diff                         # show the changes in detail
git add lib/control_system/src/control_system.c   # stage a file
git commit -m "Read encoder angles in control loop"
git push                         # upload to GitHub
git pull                         # download from GitHub
```

## Marking a finished exercise with a tag

When an exercise is done, mark the last commit with a tag, e.g. `exercise-02`. Later you can always go back to exactly this state, and your lecturers know which version to look at.

**In VS Code:**

1. Stage, commit and push your solution (see above).
2. Open the **Command Palette** (`Ctrl+Shift+P`), run **Git: Create Tag**, enter `exercise-02` and press Enter twice (the tag message is optional).
3. Open the **Command Palette** again and run **Git: Push Tags**.

**In the terminal:**

```bash
git tag exercise-02
git push origin exercise-02
```

Check on GitHub: on the page of your repository, click on the branch selector (it shows `main`) and switch to the **Tags** tab.

> [!WARNING]
> A tag that has not been pushed exists only on your computer. **Don't forget Git: Push Tags.**

### Looking at an earlier solution

Click the branch name (`main`) at the left end of the status bar at the bottom of VS Code and select the tag, e.g. `exercise-02`. Your files now show the state of this tag.

To continue working, switch back to `main` the same way.

> [!CAUTION]
> Do not change any files while a tag is selected. Changes made there don't belong to any branch and are easily lost. Always switch back to `main` first.

## Optional: trying something out on a branch

If you want to try an idea without risking your working code, use a branch:

1. Click the branch name (`main`) in the status bar and select **Create new branch...**. Enter a name, e.g. `try-new-gains`.
2. Work and commit as usual. When you push for the first time, VS Code asks whether to publish the branch: click **OK** / **Publish Branch**.
3. **The idea worked:** switch back to `main`, open the **Command Palette**, run **Git: Merge...** (in some versions **Git: Merge Branch...**) and select your branch. Then push.
4. **The idea didn't work:** simply switch back to `main`. Your working code is untouched.

## Working on more than one computer

1. Clone your repository on the second computer ([Quick Start](../README.md#quick-start), Step 3).
2. Create `include/wifi_secrets.h` again. It is not on GitHub, on purpose.
3. **Always pull (Sync Changes) before you start working, and push when you are done.** Otherwise you end up with two different versions of your code.

## Common problems

### "Please tell me who you are" / "Make sure you configure your user.name and user.email"

Git doesn't know your name yet. Run the two `git config` commands from [Getting Started](GETTING_STARTED.md#step-2-git), Step 2.

### Push rejected: "Updates were rejected because the remote contains work that you do not have locally"

GitHub has commits that your computer doesn't have yet, e.g. from your other computer or from an edit on the GitHub website. Click **Sync Changes** (or run `git pull`, then `git push`).

### Merge conflict

A conflict happens when the same lines were changed in two places (e.g. on two computers). VS Code marks the conflicting files with a **!** in the Source Control view. In the file you see both versions:

```text
<<<<<<< HEAD
your version
=======
the other version
>>>>>>> ...
```

Above each conflict, VS Code offers **Accept Current Change**, **Accept Incoming Change** and **Accept Both Changes**. Choose the right version, make sure the code is correct, save the file, stage it and commit.

### I want to undo my changes to a file

In the Source Control view, hover over the file and click the **Discard Changes** icon (curved arrow).

> [!WARNING]
> Discarded changes are gone for good.

### I want to undo my last commit (not pushed yet)

In the Source Control view, click **...** &rarr; **Commit** &rarr; **Undo Last Commit**. Your changes are kept, only the commit is removed. Fix what you need, stage and commit again.

### I tagged the wrong commit

Move the tag to the current commit and overwrite it on GitHub:

```bash
git tag -f exercise-02
git push -f origin exercise-02
```

### A pull request from "dependabot" appeared on GitHub

Dependabot is a GitHub bot that suggests updates for the automation of the original template. You can close the pull request without merging it. To stop these pull requests, delete the `.github` folder in your repository: it is only used by the template repository.
