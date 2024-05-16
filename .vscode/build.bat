if "%1"=="-my" (goto :a)
goto :b

:a
cd build & cmake .. -DCMAKE_BUILD_TYPE=Debug & cmake --build . --target ehjEngineMY -j & cd..
goto :end

:b
cd build & cmake .. -DCMAKE_BUILD_TYPE=Debug & cmake --build . --target ehjEngine -j & cd..

:end
exit
