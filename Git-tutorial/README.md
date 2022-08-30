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

So far we have only been working on a repo with git locally. However, when we need to collaborate with other people across the world, we need a way to "communicate" our progress through the network. Also it's safer to always backup the development progress on the remote git repo, in case if our computers break or are lost. (Hopefully not)

We need to first create a new github repo and upload the local one we have been working on so far, to sync. 

We name the remote repo also `myproject` for convenience.

1. Sync with Remote Repo

In our local project repo, we need to add a remote tracking link:

```console
$ git remote add https://github.com/[username]/myproject.git
```

and we check if we are remotely sync with the repo on github through `git remote -v`

```console
$ git remote -v
origin	https://github.com/[username]/myproject.git (fetch)
origin	https://github.com/[username]/myproject.git (push)
```

And then we could push all our local developments to the remote repo through:

```console
$ git push --set-upstream origin master
 
Enumerating objects: 34, done.
Counting objects: 100% (34/34), done.
Delta compression using up to 8 threads
Compressing objects: 100% (33/33), done.
Writing objects: 100% (34/34), 49.37 KiB | 12.34 MiB/s, done.
Total 34 (delta 17), reused 0 (delta 0), pack-reused 0
remote: Resolving deltas: 100% (17/17), done.
To https://github.com/[username]/myproject.git
 * [new branch]      master -> master
Branch 'master' set up to track remote branch 'master' from 'origin'.
```

Now we have uploaded all the developments to the remote repo.

2. Grab New Stuff

Now suppose something new happens on the remote repo. Here we simulate that by remove a line from the `README.md` directly on the github and commit there.

How could we update the local repo to include the changes?

We have two choices:

- `fetch` + `merge`
- `pull`

Let's start with the first combination. We need to first download all the new changes from remote using `fetch`

```console
$ git fetch origin
remote: Enumerating objects: 5, done.
remote: Counting objects: 100% (5/5), done.
remote: Compressing objects: 100% (1/1), done.
remote: Total 3 (delta 2), reused 2 (delta 2), pack-reused 0
Unpacking objects: 100% (3/3), 689 bytes | 344.00 KiB/s, done.
From https://github.com/[username]/myproject
   d4bf700..6d4ad42  master     -> origin/master
```

now we have the newly changes from upstream, we check the git status:

```console
$ git status
On branch master
Your branch is behind 'origin/master' by 1 commit, and can be fast-forwarded.
  (use "git pull" to update your local branch)
```

Clearly it shows we see that local repo is 1 commit behind the upstream master, the one line change on `README.md`. We could double check the exact difference using the `git diff` command

```console
$ git diff origin/master
diff --git a/README.md b/README.md
index f3fa9a9..cf28200 100644
--- a/README.md
+++ b/README.md
@@ -3,3 +3,5 @@ Hello World repository for Git tutorial
 This is an example repository for the Git tutoial on https://www.w3schools.com
 
 This repository is built step by step in the tutorial.
+
+a new line
```

This looks align with our expectation. Now we can merge the commit:

```console
$ git merge origin/master
Updating d4bf700..6d4ad42
Fast-forward
 README.md | 2 --
 1 file changed, 2 deletions(-)

$ git status
On branch master
Your branch is up to date with 'origin/master'.
```

There we go! Now our local git is up to date.

The above approach works, but a bit tedious. We could actually just do this update in one step by `pull` command, which is the combination of `fetch` and `merge`.

Let's add back the new line deleted from `README.md` on the github remotely and try to pull the changes to local git.

```console
$ git pull origin
remote: Enumerating objects: 5, done.
remote: Counting objects: 100% (5/5), done.
remote: Compressing objects: 100% (3/3), done.
remote: Total 3 (delta 2), reused 0 (delta 0), pack-reused 0
Unpacking objects: 100% (3/3), 696 bytes | 232.00 KiB/s, done.
From https://github.com/[username]/myproject
   6d4ad42..b2bb9ae  master     -> origin/master
Updating 6d4ad42..b2bb9ae
Fast-forward
 README.md | 2 ++
 1 file changed, 2 insertions(+)
```

and we can see we are updated with the remote master branch:

```console
$ git log --oneline
b2bb9ae (HEAD -> master, origin/master) Add back a new line from github directly
6d4ad42 Remove the new line README.md  GitHub directly
d4bf700 Update: README.md (beautiful)
327ae72 Add file3
8b159b4 Add file2
cf9f3bf Add file1
...(more)...
```

3. Push New Stuff

In last section, we learned how to download and keep in sync with remote repo. Now we are going to make our contribution to the remote repo through `push` command.

Let's just add another line in the `README.md` and try to update the remote repo.

```console
$ git commit -a -m "Update readme locally and try push"
[master f8986b8] Update readme locally and try push
 1 file changed, 1 insertion(+)

$ git status
On branch master
Your branch is ahead of 'origin/master' by 1 commit.
  (use "git push" to publish your local commits)
```

Git shows that we are 1 commit ahead of the remote master branch. Let's push the change to remote repo.

```console
$ git push origin
```

4. Remote Branch Pull

We can also directly create a new branch on the remote repo through the GUI. We create a new branch called `secondary` from `master` branch and made some changes on the `README.md`, committed on github directly.

We can pull the new branch to local git and check it:

```console
$ git pull
remote: Enumerating objects: 5, done.
remote: Counting objects: 100% (5/5), done.
remote: Compressing objects: 100% (3/3), done.
remote: Total 3 (delta 2), reused 0 (delta 0), pack-reused 0
Unpacking objects: 100% (3/3), 697 bytes | 232.00 KiB/s, done.
From https://github.com/[username]/myproject
 * [new branch]      secondary  -> origin/secondary
Already up to date.
```

We can check the new branch as usual through `git branch`. But by default it only displays local branches. We need the `-a` flag for all local and remote branches, or `-r` flag for only remote branches.

```console
$ git branch // only local branches
* master

$ git branch -a
* master
  remotes/origin/master
  remotes/origin/secondary

$ git branch -r 
  origin/master
  origin/secondary
```

5. Push Local Branch to Remote

We can push a local new branch with changes to remote repoo as well. Let's do it


```console
$ git checkout -b local-new-branch

...
(do some changes to README.md)
...
```

``` console
$ git commit -a -m "Update from local-new-branch"
[local-new-branch 40e9ee3] Update: local-new-branch
 1 file changed, 2 insertions(+)
```

```console
$ git push origin local-new-brancch
Enumerating objects: 5, done.
Counting objects: 100% (5/5), done.
Delta compression using up to 8 threads
Compressing objects: 100% (3/3), done.
Writing objects: 100% (3/3), 334 bytes | 334.00 KiB/s, done.
Total 3 (delta 2), reused 0 (delta 0), pack-reused 0
remote: Resolving deltas: 100% (2/2), completed with 2 local objects.
remote: 
remote: Create a pull request for 'local-new-branch' on GitHub by visiting:
remote:      https://github.com/[username]/myproject/pull/new/local-new-branch
remote: 
To https://github.com/[username]/myproject.git
 * [new branch]      local-new-branch -> local-new-branch
```

Now if we go to the github remote repo, we will see there is a newly pushed branch called `local-new-branch`

6. Merge Into Master

From the above, on the github repo page, we want to **merge** our changes in the `local-new-branch` into the main `master` branch. The webpage has the option **Compare & Pull Request**.

We can go click it and create a **pull request** from it. 

Since this is our own repo, we are the "authority" and can directly click **Merge pull request** to faciliate the changes merge into `master` branch. 

In real life, however, typically a pull request needs to go through **code review** and **test** process, verified by your collaborators. If approved, then will the pull request go through and get merged.

7. Fork & Clone Others' Repo

Technically speaking, `fork` is not a git command. But rather it's a functionality provided by github to directly copy another public repo and make it into your own git repo. It's widely used in open-source project, where people fork the original repo and later try to make pull request to merge their changes into "THE repo" from their own forked copy of the repo.

Typically, once we fork the repo, we want to download a local copy to our computer and start working on that. Given the url of the repo `[the repo url]`, we could use the `clone` command:

```console
$ git clone [the repo url] [the folder path we want to clone into]
```

`clone` will preserve all logging and versions of files from upstream.

Typically, we want to name the original repo as `upstream` and our fork copy as `origin`.

We can check all the remote tracking status by `git remote -v`. And we now will rename the `origin` to `upstream`:

```console
$ git remote rename origin upstream
```

And we can add tracking of a remote repo as `origin` with `[The url]` by the following command:

```console
$ git remote add origin [The url]
```

Now we have two remote trackings. If we want to make a contribution to the original repo, we could commit and push our changes to the forked repo we have, and on the github page create a **pull request** to the original repo. 

If the changes look good to code reviewers of the original repo, they will approve the pull request and merge it.

8. Git Ignore

We don't necessarily want git to track very single file in the local repo. There might be some log files, temporary files or personal files that shouldn't be included in the git workflow.

To solve this, we can create a `.gitignore` file in our git repo, and files specified in it will be ignored by our git. However, the `.gitignore` file itself is tracked by git.

Some common usages include:

| command      | Description |
| ----------- | ----------- |
|     |     blank lines are ignored  |
| # something   |lines start with # is comment and ignored        |
| *name* | All *name* files and folders |
| *name/* | All folders called *name* |
| *name.file* | All *name.file* in repo |
| **.file* | any file with extension *.file* |
| *!\*.file* | negate any previous ignore on this file |



