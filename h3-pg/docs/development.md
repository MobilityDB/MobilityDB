## Development

In order to build and test your changes, simply run `./scripts/develop`.

Documentation is generated from the sql files, using the script `scripts/documentaion` (requires poetry).

## Release Process

1. Update version number
   - Don't follow semver, simply use major and minor from H3 core and increment patch.
   - Version number should be changed in root `CMakeLists.txt`.
   - Set `INSTALL_VERSION` to "${PROJECT_VERSION}".
   - Update files (and cmake references) suffixed `--unreleased` should be renamed.
   - Installer `.sql` files should have `@ availability` comments updated.
   - Update changelog by moving from `Unreleased` to a new section
   - Push and merge changes in `release-x.y.z` branch.
2. Create a release on GitHub
   - Draft new release "vX.Y.Z"
   - Copy CHANGELOG.md entry into release description
3. Distribute the extension on PGXN
   - Run `scripts/bundle` to package the release
   - Upload the distribution on [PGXN Manager](https://manager.pgxn.org/)
4. Prepare for development
   - Set `INSTALL_VERSION` to `unreleased` in root `CMakeLists.txt`.
   - Create new update files with `--unreleased` suffix.
   - Add them to relevant `CMakeLists.txt` files.
