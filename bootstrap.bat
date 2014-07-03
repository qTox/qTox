@mkdir lib
%~dp0tools\wget --no-check-certificate http://jenkins.libtoxcore.so/job/libtoxcore-win32-i686/lastSuccessfulBuild/artifact/libtoxcore-win32-i686.zip -O %~dp0lib\libtoxcore-latest.zip
%~dp0tools\unzip %~dp0lib\libtoxcore-latest.zip -d %~dp0lib\
@del %~dp0lib\libtoxcore-latest.zip