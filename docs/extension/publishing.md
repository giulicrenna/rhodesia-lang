---
title: Editor Extension
layout: default
nav_order: 10
---

# Publishing the VS Code extension

Steps to package and publish the Rhodesia extension to the VS Code Marketplace.

The extension source lives in `extension/`. All commands below assume that as the working directory unless noted.

## Prerequisites

- **Node.js 16+** and **npm**.
- A **Microsoft / Azure DevOps account** (personal Microsoft account is fine).
- A **Marketplace publisher ID** — a short slug (e.g. `giulicrenna`), not your display name and **not your email**. The error

  ```
  Invalid extension "publisher": "you@example.com" in package.json.
  Expected the identifier of a publisher, not its human-friendly name.
  ```

  means the value in `extension/package.json` is wrong — it must be the slug you created at <https://marketplace.visualstudio.com/manage>.

## 1. Create a publisher (one time)

1. Go to <https://marketplace.visualstudio.com/manage> and sign in.
2. Click **Create publisher**.
3. Pick a unique **ID** (this is the value that goes into `package.json` as `publisher`).
4. Fill in the human-friendly name and contact email. They are *not* the same as the ID.

If the publisher already exists and you just need to find its ID, the manage page lists it.

### About Azure DevOps and the "paid organisation" prompt

Step 3 needs a Personal Access Token from Azure DevOps. There are two ways to satisfy that, depending on your account type.

#### Personal Microsoft account (outlook.com, hotmail.com, live.com)

Creating an Azure DevOps organization is **free** for personal accounts — pick the **Basic** plan when prompted ("Free for up to 5 users"). Skip "Basic + Test Plans", "Professional", and any other paid option; those are for test management and advanced pipelines, not for publishing extensions.

#### Work / school account (Microsoft Entra ID)

Since 2024, Microsoft requires every new Azure DevOps organization to be linked to an Azure subscription. If you see:

> To create an Azure DevOps organization, you need to link it to an Azure subscription. We couldn't find any subscriptions you have access to.

the workarounds are:

1. **Create a free Azure subscription** at <https://azure.microsoft.com/free/>. It requires a phone number and a credit card on file but **does not charge** — you only pay if you explicitly upgrade to pay-as-you-go. The free tier includes $200 of credit for 30 days and several always-free services. Once the subscription exists, the Azure DevOps sign-up flow proceeds normally.
2. **Sign out and use a personal Microsoft account** (outlook.com, hotmail.com, etc.). Personal accounts still have the free path.
3. **Generate a PAT through the legacy Visual Studio profile** at <https://app.vsaex.visualstudio.com/me>. Deprecated, but the page still works for some accounts. The PAT you get there is accepted by `vsce` exactly the same way.

#### Where the PAT lives

Once the org exists (whichever path you took), the PAT is at:

```
https://dev.azure.com/<your-org>/_usersSettings/tokens
```

or from the UI: click the user icon in the top-right → **Personal Access Tokens** → **+ New Token**.

## 2. Verify the package metadata

Open `extension/package.json` and confirm:

```json
{
  "name": "RhodesiaLanguage",
  "displayName": "Rhodesia Language Support",
  "publisher": "GiulianoCrenna",
  "version": "0.1.2",
  "engines": { "vscode": "^1.79.0" }
}
```

- `publisher` must match the slug from step 1.
- `version` follows semver; bump it on every release.
- `engines.vscode` should be at least the minimum version you have tested.

## 3. Install the publisher token locally

`vsce` uses a Personal Access Token (PAT) from Azure DevOps.

1. Visit <https://dev.azure.com/> → user settings → **Personal Access Tokens**.
2. Click **New Token**:
   - Name: `vsce publish` (or anything you can recognise)
   - Organization: **All accessible organizations**
   - Scopes: **Marketplace (Manage)** — that's the only scope needed.
   - Expiration: pick something short (90 days is the max). Rotate before it expires.
3. Copy the token immediately — Azure won't show it again.

Log in once per machine. The token is stored by `vsce` and reused for subsequent publishes.

```bash
cd extension
npx vsce login <your-publisher-id>
# paste the PAT when prompted
```

If `vsce` complains about the publisher identifier, you are passing your display name or email — pass the slug.

## 4. Smoke-test the package locally

Before publishing, build the `.vsix` and install it in a clean VS Code instance to make sure everything works.

```bash
cd extension
npm install          # only needed once per machine, populates node_modules
npx vsce package     # writes rhodesia-language-<version>.vsix in the current dir
```

Then, in a separate VS Code window:

```bash
code --install-extension rhodesia-language-<version>.vsix
```

Open `extension/test_extension.rho` and check: syntax highlighting, hover on `math.zeros`, signature help on `zeros(`, autocomplete with `.` and ` `, go-to-definition on `sin`, format on save (after enabling `editor.formatOnSave` in the file's settings).

## 5. Publish

When the package looks right:

```bash
cd extension
npx vsce publish
```

`vsce` will:

1. Re-package the extension.
2. Upload it to the Marketplace under the publisher from step 1.
3. Bump nothing automatically — bump `version` in `package.json` yourself between publishes.

For a one-shot publish with an explicit version bump:

```bash
npx vsce publish 0.1.1
# or with a pre-release tag
npx vsce publish 0.2.0 --pre-release
```

The new version goes live on the Marketplace within a few minutes. Anyone with VS Code will see the update notification on their next launch (or sooner if they hit *Extensions* → *Update*).

## 6. CI release (optional but recommended)

Manual publishing works. For a reproducible release, automate it with a GitHub Actions workflow that runs on a version tag.

Create `.github/workflows/publish-extension.yml`:

```yaml
name: Publish VS Code extension

on:
  push:
    tags: ['v*']

jobs:
  publish:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: actions/setup-node@v4
        with:
          node-version: 20
      - name: Install vsce
        run: npm install -g @vscode/vsce
      - name: Publish
        env:
          VSCE_PAT: ${{ secrets.VSCE_PAT }}
        working-directory: extension
        run: |
          npm install
          vsce publish
```

Add the PAT as a repository secret named `VSCE_PAT` (*Settings → Secrets and variables → Actions → New repository secret*). Then:

```bash
git tag v0.1.1
git push origin v0.1.1
```

…the workflow packages and publishes the version that matches the tag.

## 7. Unpublish / yank a release

If you push a broken version:

1. Go to <https://marketplace.visualstudio.com/manage>.
2. Pick the publisher, then the extension, then the version.
3. Click **Unpublish**.

The Marketplace removes the version from new installs and within a few hours from existing installs (VS Code re-checks on launch). There is no soft-delete — once unpublished, the version cannot be re-served; you'd have to publish a new fixed version.

## Troubleshooting

| Symptom | Likely cause |
|---|---|
| `Invalid extension "publisher"` | You're using a display name or email instead of the slug. |
| Azure DevOps sign-up asks for a paid plan | Pick **Basic** ("Free for up to 5 users"). The paid tiers are for Test Plans and extra pipelines; not needed to publish. |
| `To create an Azure DevOps organization, you need to link it to an Azure subscription` | Microsoft now requires new orgs to be linked to an Azure subscription. Create a free Azure subscription at <https://azure.microsoft.com/free/> (no charge), or use a personal Microsoft account, or fall back to the legacy VS profile. See the section above. |
| `Personal Access Token is missing required scopes` | The PAT was created without **Marketplace (Manage)**. Re-create. |
| `Extension version already exists` | You forgot to bump `version` in `package.json`. |
| `Extension is missing a publisher` | The `publisher` field is absent from `package.json`. |
| Install succeeds but the language does not activate | File association missing. Confirm VS Code is opening the file as `rhodesia` (look at the bottom-right language indicator). |

## Reference

- [vsce CLI reference](https://github.com/microsoft/vscode-vsce)
- [Marketplace publisher management](https://code.visualstudio.com/api/working-with-extensions/publishing-extension)
- [Personal Access Tokens](https://learn.microsoft.com/azure/devops/organizations/accounts/use-personal-access-tokens-to-authenticate)
