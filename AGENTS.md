# Light Warrior Codex Handoff

## GitHub Auth

- This repo pushes to `https://github.com/Nixxed09/light-warrior.git`.
- Git HTTPS credentials are provided by GitHub CLI through `gh auth git-credential`.
- `git config user.name` is only commit author metadata; it does not choose the push account.
- Before pushing, verify the active GitHub CLI account:

```powershell
gh auth status --active
```

- If the active account is not `Nixxed09`, switch it before pushing:

```powershell
gh auth switch -h github.com -u Nixxed09
```

- `Traviseric` may also be logged in, but pushing this repo as `Traviseric` can fail with a GitHub 403.

## Repo State Checks

Use an explicit path in PowerShell because some shells may start in `D:\TE-Code`:

```powershell
Set-Location -LiteralPath D:\Phoenix\nix-code\games\light-warrior
git status --short --branch
```
