---
name: github-commit
description: 'Prepare and create a git commit in the Arm Semihosting repository, following the project''s commit message conventions, and add the closing comment to the GitHub issue it fixes. Use when the user asks to "commit", "make a commit", "commit the fix", or to write a commit message for the current changes.'
argument-hint: 'Optional: the GitHub issue number (#N), and whether the commit completes the issue'
---

# Commit — Arm Semihosting

## When to Use

- The user asks to commit the current changes, or a subset of them.
- The user asks for a commit message for the staged or unstaged
  changes.

This skill is the reference for the commit messages and the issue
updates; the "Version Control" section of `.github/copilot-instructions.md`
requires descriptive commit messages, and this skill defines them. Each
commit requires the user approval.

## Procedure

1. **Inspect the working tree.** Run `git status` and `git diff` (and
   `git diff --cached` if anything is staged). Identify which changes
   belong to this commit; the working tree often contains unrelated
   changes and untracked files (for example `package-lock.json`, or
   notes in `maintenance/docs/`), which must not be included.
2. **Identify the issue.** Each commit refers to a GitHub issue. If the
   user did not give the number and it cannot be determined with
   certainty from the conversation, ask for it; do not guess. If no
   issue exists, offer to create one with the `create-github-issue`
   skill. If the user prefers a commit without an issue, see "Commits
   without an issue". Ask whether this commit completes the issue (and
   gets `Fixes #N`), or is one of several commits for it, unless that
   is already clear. A commit may fix several issues; it then has one
   `Fixes #N` line for each, and the subject has the number of the
   main one.
3. **Check that the tests were run** as required by the "Testing"
   section of `.github/copilot-instructions.md` (`xpm run test -C
   tests`, `xpm run test-native-clang -C tests`, and `xpm run
   test-qemu-cortex-m7f-gcc -C tests`, according to the change). If they were not run after the last
   change, say so and run them, or ask the user, before committing.
   Report failures; do not commit over them without explicit approval.
4. **Check the generated files.** If `xcdl-package.jsonc` changed, the
   generated files (`CMakeLists.txt`, `meson.build`) must be
   regenerated and included, not edited by hand. Do not commit the
   `.tgz` packages in the top folder, or the `node_modules` and
   `xpacks` folders.
5. **Draft the message** (see "Message Format") and write it to a file
   in the scratchpad folder.
6. **Show the user** the list of files to be committed and the full
   message, and wait for explicit approval. Never commit without it.
7. **Stage and commit** only the approved files, by name (no
   `git add -A` or `git add .`), and commit with the message file:

   ```sh
   git add <file> ...
   git commit -F <scratchpad>/commit-message.txt
   ```

   Do not amend, rebase, or skip the hooks (`--no-verify`) unless the
   user asks. If a hook fails, fix the cause and create the commit
   again. Do not push unless the user asks.
8. **Report** the commit hash and subject, with
   `git log -1 --format='%h %s'`. The short hash is the one printed by
   `%h`, 7 characters in this repository, as in `CHANGELOG.md`; use it everywhere.
9. **Update the issue**, if the commit has `Fixes #N` (see "Issue
   Comment"); the comment is posted after the commit is pushed.
10. **Ask whether to update the review documents** that list the issue
    or the finding it came from (see "Review Documents").

Do not start the next fix while this one is uncommitted; commit first.

## Message Format

### Subject

- `#N: ` followed by a summary in the imperative mood, starting with a
  capital letter: `#12: Retry the partial writes in write()`.
- At most 50 characters, including the `#N: ` prefix.
- No period at the end.
- Describe the change, not the problem (the issue title describes the
  problem): `Retry the partial writes`, not
  `The partial writes are lost`.

### Body

Separated from the subject by a blank line, wrapped at 72 characters,
in British English, formal, with no contractions and with the Oxford
comma. Identifiers, files, and options are written plainly, without
backticks. Explain what and why, not how; usually three short
paragraphs:

1. **The previous state**, in the past tense: what the code did, and
   why it was a problem.

   > When the host accepted only part of the buffer, the trace write
   > returned the shorter count, and the remaining bytes of the line
   > were lost.

2. **The change**, in the imperative: what was added, removed, or
   changed, and the decisions that are not obvious from the diff
   (what was deliberately left unchanged, and why).

   > Repeat the call until all the bytes are written, and return
   > the number of bytes actually written.

3. **Tests and documentation**, in the imperative: the tests added or
   changed, and the documentation updated.

   > Add a test in the tests folder and update the Doxygen comments.

A trivial change may have a single paragraph.

Mention the related issues and pull requests, if any, by number, where
they are relevant in the text: an earlier decision or change that this
commit builds on or reverts (`as #7 chose`, `removed by #9`), or a
follow-up left to another issue (`see #51`). Refer to an item of a
review document by the document name and the item number, so that it
is not mistaken for an issue: `(API-REVIEW-2.md #43)`.

### Trailers

After the body, separated by a blank line:

- `Fixes #N` — only on the commit that completes the issue; omit it
  from the intermediate commits of an issue fixed in several commits.
- The attribution trailer required by the agent's environment, if any
  (for example `Co-Authored-By: ...`), copied exactly as given there,
  as the last line. The example below omits it, because it depends on
  the environment; it is not optional when the environment requires
  it.

### Example

```text
#12: Retry the partial writes in write()

When the host accepted only part of the buffer, SYS_WRITE returned the
number of bytes not written, and the trace write returned the shorter
count, so the remaining bytes of the line were lost.

Repeat the call until all the bytes are written, and return the number
of bytes actually written.

Update the Doxygen comments of write().

Fixes #12
```

## Commits without an Issue

When the user decides that a change needs no issue (for example a
documentation update or a website change):

- The subject has no `#N: ` prefix; the other subject rules apply (the
  imperative mood, a capital letter, at most 50 characters, no period).
- The message has no `Fixes` trailer, and there is no issue comment.
- Ask whether a review document lists the change (see "Review
  Documents"); it is then recorded with the commit hash only.

## Issue Comment

After a commit with `Fixes #N`, add a comment to the issue. GitHub can
link a commit hash only after the commit is pushed, and an amend or a
rebase changes it; therefore post the comment after the push, with the
final hash. If the user wants the comment before the push, say that
the hash is provisional, and correct the comment after any amend:

```sh
gh issue comment <N> --body-file <scratchpad>/issue-comment.md
```

- Start with `Fixed in <short hash>`, followed by a brief explanation
  of the changes, written for the reader of the issue: what was
  changed, what was deliberately not changed and why, and how it was
  verified (measurements, compared outputs).
- Write the commit hashes bare, without backticks (`Fixed in 123f0c7`),
  so that GitHub links them to the commits; in backticks they are not
  linked. (The review documents are read as files in the repository, so
  they write the hashes in backticks; do not make the two consistent.)
- If the proposal of the issue was not followed, or the finding was
  withdrawn, say so and explain why.
- Write the paragraphs unwrapped (one line each), in Markdown, with
  backticks for identifiers (but not for the commit hashes); short
  lists are acceptable.
- End with a separate line `Fixed on YYYY-MM-DD.`, with the date of the
  commit (`git log -1 --format=%cs`).
- Do not close the issue; it is closed when the commit reaches the
  default branch (`xpack`; the development happens on `xpack-development`).

Show the comment to the user before posting it, unless the user has
already approved posting it together with the commit.

## Review Documents

The findings that lead to issues may be recorded in review, change, or
comparison documents in `maintenance/docs/` (currently only
`README-MAINTAINER.md` exists there), or in files named `*-review*`,
`*-changes*`, `*-compatibility*`, or similar. If there are none, skip
this section. After the commit, search them for the issue and the finding.
When the finding came from a known document (for example, the issue was
created from `API-REVIEW-1.md`), search that document first, by the
words of the issue title and by the item number; otherwise search all
the documents:

```sh
grep -n -i -E '<words from the issue title>|\(#<N>\)|issue #<N>' \
  maintenance/docs/*.md
```

Replace `<N>` with the number of the issue. In some documents, a `#K`
at the start of an item is the number of the finding within the
document, not a GitHub issue number; match the
GitHub number where it is used as such (in parentheses, or after
"issue" or "fixed in"), and confirm the item by its content.

If an item refers to the issue or to the same finding, **ask the user**
whether to update it, and show the proposed text. Then update the item
in the format already used by that document, and always mention both
the GitHub issue number and the short commit hash, for example:

- `` Fixed in `a8733dc` (#96): <what was done>. `` (the format of
  `API-REVIEW-<n>.md`, under the item, with `(fixed)` added after its
  tags, such as `[internal] **[confirmed]**`);
- `` Status: fixed in #12, commit `a8733dc`, on 2026-11-14. <what was
  done>. `` (the format for documents without a prescribed one).

Summarise what was done, and state where the fix differs from the
recommendation of the document. For an intermediate commit of an issue
fixed in several commits, ask whether to record it as partial (for
example `Partly fixed in ... (#N): ...`). For a withdrawn finding, write
`Withdrawn (#N): <reason>.` instead.

Do not change the header date or commit of the document, which record
when the review was made. These documents are often untracked or have
unrelated changes; do not stage or commit them unless the user asks.
