package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"os/user"
	"syscall"

	"github.com/kardianos/osext"
)

var custom_user string

func fs_type(path string) int {
	//name := "FileOrDir"
	f, err := os.Open(path)
	if err != nil {
		fmt.Println(err)
		return -1
	}
	defer f.Close()
	fi, err := f.Stat()
	if err != nil {
		fmt.Println(err)
		return -1
	}
	switch mode := fi.Mode(); {
	case mode.IsDir():
		return 0
	case mode.IsRegular():
		return 1
	}

	return -1
}

func install(path string, pathlen int) int {
	files, _ := ioutil.ReadDir(path)

	for _, file := range files {
		if fs_type(path+file.Name()) == 1 {

			addpath := ""
			if len(path) != pathlen {
				addpath = path[pathlen:len(path)]
			}

			fmt.Print("Installing: ")
			fmt.Println("/Applications/qtox.app/Contents/" + addpath + file.Name())
			if _, err := os.Stat("/Applications/qtox.app/Contents/" + file.Name()); os.IsNotExist(err) {
				newfile := exec.Command("/usr/libexec/authopen", "-c", "-x", "-m", "drwxrwxr-x+", "/Applications/qtox.app/Contents/"+addpath+file.Name())
				newfile.Run()
			}

			cat := exec.Command("/bin/cat", path+file.Name())

			auth := exec.Command("/usr/libexec/authopen", "-w", "/Applications/qtox.app/Contents/"+addpath+file.Name())
			auth.Stdin, _ = cat.StdoutPipe()
			auth.Stdout = os.Stdout
			auth.Stderr = os.Stderr
			_ = auth.Start()
			_ = cat.Run()
			_ = auth.Wait()

		} else {
			install(path+file.Name()+"/", pathlen)
		}
	}
	return 0
}

func main() {
	syscall.Setuid(0)
	usr, e := user.Current()
	if e != nil {
		log.Fatal(e)
	}

CHECK:
	if usr.Name != "System Administrator" {
		fmt.Println("Not running as root, relaunching")

		appdir, _ := osext.Executable()
		appdir_len := len(appdir)
		sudo_path := appdir[0:(appdir_len-7)] + "qtox_sudo" //qtox_sudo is a fork of cocoasudo with all of its flags and other features stripped out

		if _, err := os.Stat(sudo_path); os.IsNotExist(err) {
			fmt.Println("Error: No qtox_sudo binary installed, falling back")
			custom_user = usr.Name
			usr.Name = "System Administrator"
			goto CHECK
		}

		relaunch := exec.Command(sudo_path, appdir, usr.Name)
		relaunch.Stdout = os.Stdout
		relaunch.Stderr = os.Stderr
		relaunch.Run()
		return

	} else {

		if len(os.Args) > 1 || custom_user != "" {

			if custom_user == "" {
				custom_user = os.Args[1]
			}

			update_dir := "/Users/" + custom_user + "/Library/Preferences/tox/update/"
			if _, err := os.Stat(update_dir); os.IsNotExist(err) {
				fmt.Println("Error: No update folder, is check for updates enabled?")
				return
			}
			fmt.Println("qTox Updater")

			killqtox := exec.Command("/usr/bin/killall", "qtox")
			_ = killqtox.Run()

			install(update_dir, len(update_dir))

			os.RemoveAll(update_dir)
			fmt.Println("Update metadata wiped, launching qTox")
			launchqtox := exec.Command("/usr/bin/open", "-b", "chat.tox.qtox")
			launchqtox.Run()

		} else {
			fmt.Println("Error: no user passed")
		}

	}
}
