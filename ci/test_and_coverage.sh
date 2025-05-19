SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# Install or reinstall dependencies
for p in  afb-libpython afb-test-py python3-pip afb-cmake-modules librp-utils-json-c-devel libevdev afb-helpers4 afb-helpers4-devel gcc g++ make cmake json-c afb-binding afb-binding-devel libmicrohttpd libdb-devel platform-runtime-tools; do
    sudo rpm -q $p && sudo dnf update -y $p || sudo dnf install -y $p 
done

sudo pip3 install libevdev

# Install lcov from GitHub
sudo dnf install -y https://github.com/linux-test-project/lcov/releases/download/v1.16/lcov-1.16-1.noarch.rpm

CMAKE_COVERAGE_OPTIONS="-DCMAKE_C_FLAGS=--coverage -DCMAKE_CXX_FLAGS=--coverage"

rm -rf build
mkdir -p build
cd build
cmake ${CMAKE_COVERAGE_OPTIONS}  ..
make VERBOSE=1 

sudo systemctl start platform-core-detection
sudo systemctl start platform-os-detection
sudo systemctl start platform-devices-detection

LD_LIBRARY_PATH=. python ../tests/test-basic.py -vvv --tap

sudo chmod 0666 /dev/uinput
sudo chown root:input /dev/uinput
sudo systemctl restart systemd-udevd

LD_LIBRARY_PATH=. python ../tests/test-event.py -vvv --tap

#
# Coverage
#
cd ..

ls -la build/

# Debug
echo "Coverage files:"
find build -name "*.gcda" -o -name "*.gcno"

rm -f lcov_cobertura*
wget https://github.com/eriwen/lcov-to-cobertura-xml/releases/download/v2.1.1/lcov_cobertura-2.1.1.tar.gz
tar xzvf lcov_cobertura*tar.gz

rm app.info
lcov --directory . --capture --output-file app.info
# remove system headers from coverage
lcov --remove app.info '/usr/*' -o app_filtered.info
# output summary (for Gitlab CI coverage summary)
lcov --list app_filtered.info
# generate a report (for source annotation in MR)
PYTHONPATH=./lcov_cobertura-2.1.1/ python -m lcov_cobertura app_filtered.info -o ./coverage.xml

genhtml -o html app_filtered.info
