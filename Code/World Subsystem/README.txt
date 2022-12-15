A UWorldSubsystem with some useful QOL features:

* Controllable network initialization (e.g, Client or Server-only subsystems)
* Level-Specific Initialization (e.g, skip in MainMenu, Entry etc.)
* Virtual stub to work around the infinitely frustrating initialization-order differences/issues in PIE vs Standalone
* A level based tick function, rather than FTickableGameObject interface.