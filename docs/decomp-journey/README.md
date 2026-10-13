# Decomp journey

Notes from learning game decompilation on this project, starting from zero. They were written while working with an AI assistant (Claude); see the commit trailers.

| File | What it is |
|---|---|
| [`game-decompilation-guide.md`](game-decompilation-guide.md) | Background: how matching decompilation works, Twilight Princess and the Dusklight PC port, static recompilation (NWiiRecomp) |
| [`QUICKSTART.md`](QUICKSTART.md) | Setting up this repo, every new concept explained, and three fully worked examples (sections 4–6) |
| [`FINDINGS.md`](FINDINGS.md) | Technical results: new symbol names, class layouts, the stage reverb presets, compiler quirks, open problems |

The code from the worked examples is on three independent branches, each based directly on `main`:

1. `shutter-fence-forbiddance`: one function, unit linked as `Matching`
2. `ship-flood`: a whole actor, 99.89% (one function still `NONMATCHING`)
3. `stage-reverb`: a file with no prior source, 0% → 100%, linked as `Matching`

Paths like `<workdir>` refer to the folder that contains the `ss/` checkout and the disc image.
