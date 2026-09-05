# Trocto

The contract language compiler for the Astrolune ALVM.

Trocto is a high-level smart contract language that compiles to Regol IR and then to ALVM bytecode. It supports:

- **v0.2**: Constructors, string literals, maps (address/u64 keys), assert, imports
- **v0.3**: Enums, structs, events, `only_owner` access control, member access

## Build

```bash
cmake --preset dev
cmake --build --preset dev
```

## Usage

```bash
# Compile a contract
trocto contract.tc -o contract.bin

# Emit Regol IR
trocto contract.tc --emit-regol
```

## License

MIT
