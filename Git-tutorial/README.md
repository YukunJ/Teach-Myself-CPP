## Quick Tutorial of Git

**Git** is the version control tool that almost every programmer needs to know and use it proficiently. I always have problems using git when collaborating with other people. Therefore, here comes a brief quick tutorial of common git commands and workflows with github. Some may also come with cute illustrations.

+ Main reference: [W3Schools Git Tutorial](https://www.w3schools.com/git/)

Our tutorial will split into two parts: **git local** operation and **git remote** operation.

---

### Git Local
1. Check the Version 

First of all, we want to make sure **Git** is properly installed on the system.
```console
$ git --version
git version 2.34.1
```

2. Configure Git

We typically configure git to use the username/email/password to the ones we register at github. The ```global``` keyword setup the config for every repository on the system. We can remove the ```global``` keyword to setup only for current repository.
```console
// how to setup config
$ git config --global user.name "Tommy"
$ git config --global user.email "tommy@gmail.com"
$ git config --global user.password "hello123"
```

```console
// how to retrieve config
$ git config user.name
Tommy
$ git config user.email
tommy@gmail.com
$ git conig user.password
hello123
```

3. Initialize Git 

We can create a new repo and initialize git to track a folder as follows:
```console
$ mkdir myproject && cd myproject
$ git init
hint: Using 'master' as the name for the initial branch. This default branch name
hint: is subject to change. To configure the initial branch name to use in all
hint: of your new repositories, which will suppress this warning, call:
hint: 
hint: 	git config --global init.defaultBranch <name>
hint: 
hint: Names commonly chosen instead of 'master' are 'main', 'trunk' and
hint: 'development'. The just-created branch can be renamed via this command:
hint: 
hint: 	git branch -m <name>
Initialized empty Git repository in ./Git-tutorial/myproject/.git/
```

We can observe that the hidden folder `/.git/` is where git store all the trackings it does for us.

4. Check Status

We create a new file called `index.html` in our project repo. We can see the git spots this new file:
```console
$ git status
On branch master

No commits yet

Untracked files:
  (use "git add <file>..." to include in what will be committed)
	index.html

nothing added to commit but untracked files present (use "git add" to track)
```

There is shorthand `git status --short` to show a brief summary of status. The flags are `??` for untracked file, `A` for files added to stage, `M` for modified files and `D` for deleted file.

5. Stage New Changes

We saw git says we have some untracked files.

In git, every file can be in one of the two stages:

1. *Tracked* - git is actively monitoring any changes on that as part of the repo
2. *Untracked* - although the file exist, git "ignore" its existence and change

When we hit a milestone working on a file, we can **stage** the file. It will be ready to be **commit** later on. 

```console
$ git add index.html
$ git status
On branch master

No commits yet

Changes to be committed:
  (use "git rm --cached <file>..." to unstage)
	new file:   index.html
```

We can see after we stage this file and check the git status, the status goes from `untracked` to `to be committed`.

There are *shortcuts* for staging multiple files all at once. Either `git add --all` or `git add -A` would work to stage any changes in current repo.

6. Commit Changes

If we are to make a metaphor, staging is we tell git to "please keep an eye on my files in this repo" and commit is we tell git to "please take a photo **snapshot** of this repo now".

Commit is a "save point" that we may later return back to if we find any bugs in development in later on. As a good practice, we should also try to leave a meaningful message for every commit, so that future-us could know what's exactly included in this commit. The command is as follows:

```console
$ git commit -m "First release of Git Helloworld Project"
[master (root-commit) 48d7a59] First release of Git Helloworld Project
 3 files changed, 26 insertions(+)
 create mode 100644 README.md
 create mode 100644 bluestyle.css
 create mode 100644 index.html
```

Although it's not recommended, we could also bring a change into commit in one step, skipping the staging. The shortcut command is `git commit -a -m [commit message]`.

7. Commit History

We can view the commit history so far on this repo by using the `log` command:

```console
$ git log
commit 2daa2870f5b893fd8c89dedcfb9c0f9151b4413d (HEAD -> master)
Author: Tommy <tommy@gmail.com>
Date:   Mon Aug 29 19:56:48 2022 -0400

    Another changes

commit 48d7a59c0e8b12f9a4467b9f156d9c1af488d866
Author: Tommy <tommy@gmail.com>
Date:   Mon Aug 29 19:50:34 2022 -0400

    First release of Git Helloworld Project
```

8. Git Help

There are so many flag options and commands that we are not expected to memorize.

`git help --all` will show all available git commands.

`git [command] -help` will show all the available flags to be set with this *command*, and `git [command] --help` will open up the manual for this *command*. (single slash v.s. double slash)

9. Git Branch

Branches in git allow us to work on different part of a repo without affecting the main branch. 

We can create a new branch:

```console
$ git branch hello-world-image
```

check all available branches on this repo:

```console
$ git branch
  hello-world-image
* master
```

and then we will move from the master branch to the newly created branch through `checkout` command:

```console
$ git checkout hello-world-image
Switched to branch 'hello-world-image'
```

(The creation and switch to of a new branch could be done in one step by shortcut `git checkout -b hello-world-image`)

Now let's add a new image `img_hello_world.png` to the repo and commit it.

```console
$ git add --all
$ git commit -m "Added image to Hello World"
```

We can see now that there is 4 files in the folder:

```console
$ ls
README.md		img_hello_world.png
bluestyle.css		index.html
```

and if we switch back to `master` branch, the new image is not part of it. Our new changes in another branch doesn't impact the `master` branch.

```console
$ git checkout master
Switched to branch 'master'
$ ls
README.md	bluestyle.css	index.html
```

10. Merge Branch

Suppose now we are very happy with our new development in the `hello-world-image` branch and decide to merge it back to the `master` branch.

Since we are making the merge change to `master` branch, we need to firt make sure we stand on the `master` branch:

```console
$ git checkout master
Switched to branch 'master'
```

and then we apply the `merge` command:

```console
$ git merge hello-world-image
Updating 2daa287..c31a2c0
Fast-forward
 img_hello_world.png | Bin 0 -> 48630 bytes
 1 file changed, 0 insertions(+), 0 deletions(-)
 create mode 100644 img_hello_world.png
```

Success! We see the git says this merge is "Fast-forward" since it didn't see any confict with these two branches and merge happens smoothly.

However, life is not always so easy. Sometimes **merge conflict** happens. 

Suppose now we go back to `hello-world-image` and add a new line in `index.html`. Meanwhile we delete a line in `index.html` in the `master` branch, and commit changes in both branches.

and now when we try to merge this commit with the `master` branch, a conflict happens:

```console
$ git merge hello-world-image
Auto-merging index.html
CONFLICT (content): Merge conflict in index.html
Automatic merge failed; fix conflicts and then commit the result.
```

we can open `index.html` and see git has left some marks about why and how conflicts happen there:

```console
<!DOCTYPE html>
<html>
<head>
<title>Hello World!</title>
<link rel="stylesheet" href="bluestyle.css">
</head>
<body>

<h1>Hello world!</h1>
<p>This is the first file in my new Git Repo.</p>
<<<<<<< HEAD
=======

<p>A old line in our file!</p>
>>>>>>> hello-world-image
</body>
</html>
```

We can choose to keep everything between `<<<<<<< HEAD` to `=======`, or everything between `=======` to `>>>>>>> hello-world-image`.

After we make our choices in the conflict parts in `index.html`, we re-stage the file and commit it to conclude this merge operation.

```console
$ git add index.html
$ git commit -m "after fix the conflict"
[master bdfc2b1] after fix the conflict
```

This concludes the merge of two conflicting branches, and by passing the flag `-d`, we can delete the merged branch to keep a clean workspace:

```console
$ git branch -d hello-world-images
Deleted branch hello-world-image (was 9e7a8ee).
```

11. Revert Previous Commit

`revert` is the command to take off a previous commit and make the deletion into a new commit, without modifying with the log.

suppose we dont' want to add that line in the `index.html`. The change is included in the commit "add a line". We want to remove it.

first of all, we want to check which commit is the one we want go back to: (use `--oneline` keyword to display a concise git log)

```console
$ git log --oneline
bdfc2b1 (HEAD -> master) fix conflicts:
9e7a8ee add a line
17fd1f5 remove a line
5ec2abb a small change
c31a2c0 Added image to Hello World
2daa287 Another changes
48d7a59 First release of Git Helloworld Project
```

We can just revert back to the latest commit where we fix the merge conflcits. With `--no-edit` flag, we will get the default revert message in the log:

```console
$ git revert 9e7a8ee --no-edit
```

There are some shortcuts for revert. `git revert HEAD` will revert the latest commit, while `git revert HEAD~x` will revert the x-th latest commit (index start with 0, so `HEAD` is actually `HEAD~0`)

12. Reset

To compare with the `revert` command we mentioned above, `reset` will move the repo all the way back to a previous commit, essentially removing all the changes in between that commit and latest version. `revert` only remove changes in one previous commit but does keep changes after that commit.

Now, suppose we add two files `file1.txt`, `file2.txt` and `file3.txt` and include them in two separate commits:

```console
$ touch file1.txt file2.txt file3.txt
$ git add file1.txt && git commit -m "Add file1"
$ git add file2.txt && git commit -m "Add file2"
$ git add file3.txt && git commit -m "Add file3"
$ git log --oneline
327ae72 (HEAD -> master) Add file3
8b159b4 Add file2
cf9f3bf Add file1
...(more)...
```

And now we don't want the file2 and file3, but we do want to retain file1. We could just revert the 2 recent commits separately, or we will reset back to the commit where we add the file1.

```console
$ git reset cf9f3bf
$ git log
cf9f3bf (HEAD -> master) Add file1
...(more)...
```
**Warning** it's generally dangerous to mess around with the commit history of a repo, especially when working in collaboration with others. 

There is way to undo reset if we know the git commit hash. In previous example, even if we reset back to file1's commit, we can still go back since we know the last commit's hash where file 3 is added is `327ae72`.

```console
$ git reset 327ae72
$ git log
327ae72 (HEAD -> master) Add file3
8b159b4 Add file2
cf9f3bf Add file1
...(more)...
```

13. Amend

`commit --amend` could be used to modify the recent commit and swap change its commit message.

It combines the changes in `staging area` with the latest commit and create a new commit out of it, replacing the latest commit.

For example, we change the `README.md` a little bit and want to make a commit, but type in quite a few typos there in the commit messages.

```console
$ git add README.md
$ git commit -m "Upated: RMEADE.md (ugly typos)"
$ git log --oneline
b0dfb07 (HEAD -> master) Upated: RMEADE.md (ugly typos)
327ae72 Add file3
...(more)...
```
Oh No! The git history looks bad with our typos. No worries! `amend` comes to help.

```console
$ git commit --amend -m "Update: README.md (beautiful)"
$ git log --oneline
d4bf700 (HEAD -> master) Update: README.md (beautiful)
327ae72 Add file3
...(more)...
```

We cam see the previous typo-commit is replaced by our newly-amended one.

---

### Git Remote