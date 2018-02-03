### Build

```
mkdir -p build
cd build
cmake ..
make -j4
./executable file_name.txt

```

### Timeline

`SCC` -> `SCC_WFSCCQUEUE` (discarded) -> `SCC_WFPOOL` -> `SCC_SEM` -> `SCC_SEM_WFPOOL` -> `SCC_REMOVED_PHASE3` -> `SCC_STATIC_ASSIGNMENT` -> `SCC_MAIN_THREAD_STATIC`, `SCC_MAIN_THREAD_DYNAMIC`