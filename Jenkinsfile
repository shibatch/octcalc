pipeline {
    agent { label 'jenkinsfile' }

    stages {
        stage('Preamble') {
            parallel {
                stage('x86_64 windows vs2022') {
            	     agent { label 'windows11 && vs2022' }
                     options { skipDefaultCheckout() }
            	     steps {
                         cleanWs()
                         checkout scm
		     	 bat """
			 call "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat"
			 if not %ERRORLEVEL% == 0 exit /b %ERRORLEVEL%
			 path %PATH%;"c:\opt\qt6\bin"
			 call "winbuild-msvc.bat" -DCMAKE_PREFIX_PATH=c:/opt/qt6 -DENABLE_WIX=TRUE -DSUPPRESS_WIX_VALIDATION=TRUE
			 if not %ERRORLEVEL% == 0 exit /b %ERRORLEVEL%
			 ctest -j %NUMBER_OF_PROCESSORS% --output-on-failure
			 exit /b %ERRORLEVEL%
			 """
		     }
		}

                stage('x86_64 windows clang') {
            	     agent { label 'windows11 && vs2022' }
                     options { skipDefaultCheckout() }
            	     steps {
                         cleanWs()
                         checkout scm
		     	 bat """
			 call "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat"
			 if not %ERRORLEVEL% == 0 exit /b %ERRORLEVEL%
			 path %PATH%;"c:\opt\qt6\bin"
			 call "winbuild-clang.bat" -DCMAKE_PREFIX_PATH=c:/opt/qt6 -DENABLE_WIX=TRUE -DSUPPRESS_WIX_VALIDATION=TRUE
			 if not %ERRORLEVEL% == 0 exit /b %ERRORLEVEL%
			 ctest -j %NUMBER_OF_PROCESSORS% --output-on-failure
			 exit /b %ERRORLEVEL%
			 """
		     }
		}

                stage('x86_64 linux clang-18') {
            	     agent { label 'x86_64 && ubuntu22' }
                     options { skipDefaultCheckout() }
            	     steps {
                         cleanWs()
                         checkout scm
	    	     	 sh '''
                	 echo "x86_64 clang-18 on" `hostname`
			 export CC=clang-18
			 export CXX=clang++-18
			 rm -rf build
 			 mkdir build
			 cd build
			 cmake -GNinja -DCMAKE_INSTALL_PREFIX=../../install ..
			 cmake -E time ninja
		         export CTEST_OUTPUT_ON_FAILURE=TRUE
		         ctest -j `nproc`
			 '''
            	     }
                }

                stage('x86_64 linux gcc-11') {
            	     agent { label 'x86_64 && ubuntu22' }
                     options { skipDefaultCheckout() }
            	     steps {
                         cleanWs()
                         checkout scm
	    	     	 sh '''
                	 echo "x86_64 gcc-11 on" `hostname`
			 export CC=gcc-11
			 export CXX=g++-11
			 rm -rf build
 			 mkdir build
			 cd build
			 cmake -GNinja -DCMAKE_INSTALL_PREFIX=../../install ..
			 cmake -E time ninja
		         export CTEST_OUTPUT_ON_FAILURE=TRUE
		         ctest -j `nproc`
			 '''
            	     }
                }

                stage('aarch64 linux gcc-11') {
            	     agent { label 'aarch64 && ubuntu22' }
                     options { skipDefaultCheckout() }
            	     steps {
                         cleanWs()
                         checkout scm
	    	     	 sh '''
                	 echo "aarch64 gcc-11 on" `hostname`
			 export CC=gcc-11
			 export CXX=g++-11
			 rm -rf build
 			 mkdir build
			 cd build
			 cmake -GNinja -DCMAKE_INSTALL_PREFIX=../../install ..
			 cmake -E time ninja
		         export CTEST_OUTPUT_ON_FAILURE=TRUE
		         ctest -j `nproc`
			 '''
            	     }
                }

                stage('aarch64 linux clang-18') {
            	     agent { label 'aarch64 && ubuntu24' }
                     options { skipDefaultCheckout() }
            	     steps {
                         cleanWs()
                         checkout scm
	    	     	 sh '''
                	 echo "aarch64 clang-18 on" `hostname`
			 export CC=clang-18
			 export CXX=clang++-18
			 rm -rf build
 			 mkdir build
			 cd build
			 cmake -GNinja -DCMAKE_INSTALL_PREFIX=../../install ..
			 cmake -E time ninja
		         export CTEST_OUTPUT_ON_FAILURE=TRUE
		         ctest -j `nproc`
			 '''
            	     }
                }

                stage('aarch64 macos clang-17') {
            	     agent { label 'macos' }
                     options { skipDefaultCheckout() }
            	     steps {
                         cleanWs()
                         checkout scm
	    	     	 sh '''
			 eval "$(/opt/homebrew/bin/brew shellenv)"
			 export CC=/opt/homebrew/opt/llvm@17/bin/clang-17
			 export CXX=/opt/homebrew/opt/llvm@17/bin/clang++
			 rm -rf build
 			 mkdir build
			 cd build
			 cmake -GNinja -DCMAKE_INSTALL_PREFIX=../../install ..
			 cmake -E time ninja
		         export CTEST_OUTPUT_ON_FAILURE=TRUE
		         ctest -j `sysctl -n hw.physicalcpu`
			 '''
            	     }
                }
            }
        }
    }
}
