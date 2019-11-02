# Developer Documentation

## Installation

### Composer (for Autoloading)

Currently, the LearningNet plugin does not have any PHP dependencies.
Nonetheless, the dependency manager composer is used for its autoloading feature.
It automatically loads undefined classes and interfaces,
associating namespaces with directories as specified in the file `composer.json`.

Get [composer](https://getcomposer.org/doc/00-intro.md) and run the setup:
```bash
composer install
composer update
```

### Frontend

The frontend uses JavaScript.
It depends on "dagre-d3" and "graphlib" for graph visualization as well as
"webpack" and companions for module packing and easier development.

Install [nodejs](https://nodejs.org/en/download/package-manager/) and npm,
then use npm to download the dependencies:
```bash
npm install
npm run build:dev
```

From this point you can develop and make changes.
If you change some js or less files just call `npm run build:dev` to build new static js and css files.

If you like to have a ready to install zip file:
```bash
npm run zip
```

### Backend

The backend uses C++.
It has to be built both for development of the plugin and for use in practice.
To build the backend, both cmake and make have to be installed (doxygen for documentation).

#### Dependencies and Building

The backend has two external dependencies which are expected to in the directory
`./backend/deps/`:

* [LEMON](https://lemon.cs.elte.hu/trac/lemon/wiki/Downloads) for graph algorithms
* [RapidJSON](https://github.com/Tencent/rapidjson/) for JSON parsing

After downloading and extracting these dependencies to `./backend/deps/lemon`
and `./backend/deps/rapidjson` respectively, the executable has to be built:

```bash
cd backend
mkdir build
cd build

# run cmake to create a Makefile
# for more advanced usage and setting specific CMake environment variables,
# employ the tui via 'ccmake ..'
cmake ..
make -j4
```

To create documentation under `./backend/doc/html/index.html`, use `make doc`.

#### Executable and Tests

The executable `learningnet-pathfinder` should now be in the directory
`./backend/build`, where it will be searched for by the LearningNet plugin.
It can be called manually with a JSON string as an argument.
The following example uses the content of a test file:
```bash
./learningnet-pathfinder "$(cat ../test/resources/json/recommend_path.json)"
```

After building, you can also run the tests:
```bash
# from the directory ./backend/build/
./test/compress_test
./test/check_test
./test/recommend_test

# for usage information and more options when running tests
./test/compress_test --help
```

## Directory Structure

Directories marked with ^ have to be created by the developer.

Directories marked with ' are created by build tools.

```
.
|-- assets
|   |-- css              -- frontend css
|   |-- dist'            -- bundled js files
|   |-- images           -- screenshots
|   `-- js               -- frontend js
|-- backend
|   |-- build^           -- build dir for backend
|   |-- cmake            -- cmake instructions to find installed c++ dependencies
|   |-- deps^            -- downloaded backend dependencies
|   |   |-- lemon
|   |   `-- rapidjson
|   |-- doc              -- config for backend documentation
|   |   `-- html'        -- backend doc
|   |-- include          -- header files for learningnet (LN) library
|   |   `-- learningnet
|   |-- CMakeLists.txt   -- cmake build config
|   |-- test             -- backend tests
|   |   |-- catch        -- test library Catch
|   |   `-- resources    -- example files used in tests
|   |-- main.cpp         -- code for main executable, uses LN library
|   |-- test-main.cpp    -- makes Catch work
|   `-- README.md
|-- controllers
|   `-- net.php          -- the one and only controller
|-- locale               -- translations
|-- migrations           -- database migrations
|-- models               -- SORM classes encapsulating database
|-- node_modules'        -- JavaScript dependencies
|-- src                  -- auxiliary classes for controller
|-- vendor'              -- composer output
|-- views                -- html templates
|   `-- net
|-- composer.json        -- composer config
|-- .gitignore
|-- LearningNet.php      -- entry point for plugin
|-- package.json         -- npm config
|-- package-lock.json
|-- plugin.manifest      -- Stud.IP plugin info
|-- README.md
`-- webpack.config.js    -- webpack configuration
```
