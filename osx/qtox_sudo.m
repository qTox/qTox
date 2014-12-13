// Modifications listed here GPLv3: https://gist.githubusercontent.com/stqism/2e82352026915f8f6ab3/raw/fca6f6f16fb8d61a64b6053dacf50c3433c2e0af/gistfile1.txt
//
// Copyright 2009 Performant Design, LLC. All rights reserved.
// Copyright (C) 2014 Tox Foundation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//

#import <Cocoa/Cocoa.h>

#include <sys/stat.h>
#include <unistd.h>
#include <Security/Authorization.h>
#include <Security/AuthorizationTags.h>

char *addFileToPath(const char *path, const char *filename) {
	char *outbuf;
	char *lc;

	lc = (char *)path + strlen(path) - 1;
	
    if (lc < path || *lc != '/') {
		lc = NULL;
	}
    
	while (*filename == '/') {
		filename++;
	}
    
	outbuf = malloc(strlen(path) + strlen(filename) + 1 + (lc == NULL ? 1 : 0));
	
    sprintf(outbuf, "%s%s%s", path, (lc == NULL) ? "/" : "", filename);
	
	return outbuf;
}

int isExecFile(const char *name) {
	struct stat s;
    
	return (!access(name, X_OK) && !stat(name, &s) && S_ISREG(s.st_mode));
}

char *which(const char *filename)
{
	char *path, *p, *n;
	
	path = getenv("PATH");
	
    if (!path) {
		return NULL;
	}

	p = path = strdup(path);
	
    while (p) {
		n = strchr(p, ':');
		
        if (n) {
			*n++ = '\0';
		}
        
		if (*p != '\0') {
			p = addFileToPath(p, filename);
            
			if (isExecFile(p)) {
				free(path);
				
                return p;
			}
            
			free(p);
		}
        
		p = n;
	}
    
	free(path);
	
    return NULL;
}

int cocoaSudo(char *executable, char *commandArgs[], char *icon, char *prompt) {
	int retVal = 1;
	OSStatus status;
	AuthorizationRef authRef;
	
	AuthorizationItem right = {kAuthorizationRightExecute, 0, NULL, 0};
	AuthorizationRights rightSet = {1, &right};
	
	AuthorizationEnvironment myAuthorizationEnvironment;
	AuthorizationItem kAuthEnv[2];
	myAuthorizationEnvironment.items = kAuthEnv;

	AuthorizationFlags flags = kAuthorizationFlagDefaults;
	
	
	if (prompt && icon) {
		kAuthEnv[0].name = kAuthorizationEnvironmentPrompt;
		kAuthEnv[0].valueLength = strlen(prompt);
		kAuthEnv[0].value = prompt;
		kAuthEnv[0].flags = 0;
		
		kAuthEnv[1].name = kAuthorizationEnvironmentIcon;
		kAuthEnv[1].valueLength = strlen(icon);
		kAuthEnv[1].value = icon;
		kAuthEnv[1].flags = 0;
		
		myAuthorizationEnvironment.count = 2;
	}
	else if (prompt) {
		kAuthEnv[0].name = kAuthorizationEnvironmentPrompt;
		kAuthEnv[0].valueLength = strlen(prompt);
		kAuthEnv[0].value = prompt;
		kAuthEnv[0].flags = 0;
		
		myAuthorizationEnvironment.count = 1;
	}
	else if (icon) {
		kAuthEnv[0].name = kAuthorizationEnvironmentIcon;
		kAuthEnv[0].valueLength = strlen(icon);
		kAuthEnv[0].value = icon;
		kAuthEnv[0].flags = 0;
		
		myAuthorizationEnvironment.count = 1;
	}
	else {
		myAuthorizationEnvironment.count = 0;
	}
	
	status = AuthorizationCreate(NULL, &myAuthorizationEnvironment, flags, &authRef);
	
	if (status != errAuthorizationSuccess) {
		NSLog(@"Could not create authorization reference object.");
		status = errAuthorizationBadAddress;
	}
	else {
		flags = kAuthorizationFlagDefaults |
			kAuthorizationFlagInteractionAllowed |
			kAuthorizationFlagPreAuthorize |
			kAuthorizationFlagExtendRights;

		status = AuthorizationCopyRights(authRef, &rightSet, &myAuthorizationEnvironment, flags, NULL);
	}

	if (status == errAuthorizationSuccess) {
		FILE *ioPipe;
		char buffer[1024];
		int bytesRead;

		flags = kAuthorizationFlagDefaults;
		status = AuthorizationExecuteWithPrivileges(authRef, executable, flags, commandArgs, &ioPipe);

		/* Just pipe processes' stdout to our stdout for now; hopefully can add stdin pipe later as well */
		
        	for (;;) {
			bytesRead = fread(buffer, sizeof(char), 1024, ioPipe);
			
            		if (bytesRead < 1) {
                		break;
            		}
			
            		write(STDOUT_FILENO, buffer, bytesRead * sizeof(char));
		}
		
		pid_t pid;
		int pidStatus;
        
		do {
			pid = wait(&pidStatus);
		} 
        	while (pid != -1);
		
        	if (status == errAuthorizationSuccess) {
			retVal = 0;
		}
	}
	else {
		AuthorizationFree(authRef, kAuthorizationFlagDestroyRights);
		authRef = NULL;
		
        	if (status != errAuthorizationCanceled) {
			// pre-auth failed
			NSLog(@"Pre-auth failed");
		}
	}
	
	return retVal;
}

void usage(char *appNameFull) {
	char *appName = strrchr(appNameFull, '/');
    
	if (appName == NULL) {
		appName = appNameFull;
	}
	else {
		appName++;
	}
    
	fprintf(stderr, "usage: %s command\n", appName);
}

int main(int argc, char *argv[]) {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	int retVal = 1;
	int programArgsStartAt = 1;
	char *icon = NULL;
	char *prompt = NULL;

	if (programArgsStartAt >= argc) {
		usage(argv[0]);
	}
	else {
		char *executable;

		if (strchr(argv[programArgsStartAt], '/')) {
			executable = isExecFile(argv[programArgsStartAt]) ? strdup(argv[programArgsStartAt]) : NULL;
		}
		else {
			executable = which(argv[programArgsStartAt]);
		}

		if (executable) {
			char **commandArgs = malloc((argc - programArgsStartAt) * sizeof(char**));
			
            memcpy(commandArgs, argv + programArgsStartAt + 1, (argc - programArgsStartAt - 1) * sizeof(char**));
			
            commandArgs[argc - programArgsStartAt - 1] = NULL;
			
            retVal = cocoaSudo(executable, commandArgs, icon, prompt);
			
            free(commandArgs);
			free(executable);
		}
		else {
			fprintf(stderr, "Unable to find %s\n", argv[programArgsStartAt]);
			
            usage(argv[0]);
		}
	}

	if (prompt) {
		free(prompt);
	}
	
	[pool release];
    
	return retVal;
}
