# Publish using the GitHub website

This browser-compatible tree contains losslessly zipped STEP models and updated CAD links. Extract each `.step.zip` file before opening it in CAD.

On an update branch of the existing CLMD repository, remove the superseded `docs`, `firmware`, `hardware` and `software` directories. Upload the replacement root contents in two batches (at most 100 files each): everything except `firmware`, then the `firmware` folder. Drag the contents of the local batch folders into the repository root; do not upload the batch folder names or the outer ZIP itself. Review and merge a pull request.

After merging, create the `firmware-v2.0.3` tag on `main` and publish a release using [RELEASE_DESCRIPTION.md](RELEASE_DESCRIPTION.md). Attach the unchanged `clmd-firmware-2.0.3.zip` and its checksum from the handoff.
