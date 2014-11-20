package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"os/user"
)

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

func main() {
	usr, e := user.Current()
	if e != nil {
		log.Fatal(e)
	}

	update_dir := usr.HomeDir + "/Library/Preferences/tox/update/"
	if _, err := os.Stat(update_dir); os.IsNotExist(err) {
		fmt.Println("Error: No update folder, is check for updates enabled?")
		return
	}
	fmt.Println("qTox Updater")

	files, _ := ioutil.ReadDir(update_dir)
	killqtox := exec.Command("/usr/bin/killall", "qtox")
	_ = killqtox.Run()

	for _, file := range files {
		if fs_type(update_dir+file.Name()) == 1 {
			fmt.Print("Installing: ")
			fmt.Println("/Applications/qtox.app/Contents/" + file.Name())
			if _, err := os.Stat("/Applications/qtox.app/Contents/" + file.Name()); os.IsNotExist(err) {
				newfile := exec.Command("/usr/libexec/authopen", "-c", "-x", "-m", "drwxrwxr-x+", "/Applications/qtox.app/Contents/"+file.Name())
				newfile.Run()
			}

			cat := exec.Command("/bin/cat", update_dir+file.Name())
			auth := exec.Command("/usr/libexec/authopen", "-w", "/Applications/qtox.app/Contents/"+file.Name())
			auth.Stdin, _ = cat.StdoutPipe()
			auth.Stdout = os.Stdout
			auth.Stderr = os.Stderr
			_ = auth.Start()
			_ = cat.Run()
			_ = auth.Wait()

		} else {
			files, _ := ioutil.ReadDir(update_dir + file.Name())
			for _, file2 := range files {
				fmt.Print("Installing: ")
				fmt.Println("/Applications/qtox.app/Contents/" + file.Name() + "/" + file2.Name())

				if _, err := os.Stat("/Applications/qtox.app/Contents/" + file.Name() + "/" + file2.Name()); os.IsNotExist(err) {
					newfile := exec.Command("/usr/libexec/authopen", "-c", "-x", "-m", "drwxrwxr-x+", "/Applications/qtox.app/Contents/"+file.Name()+"/"+file2.Name())
					newfile.Run()
				}

				cat := exec.Command("/bin/cat", update_dir+file.Name()+"/"+file2.Name())
				auth := exec.Command("/usr/libexec/authopen", "-w", "/Applications/qtox.app/Contents/"+file.Name()+"/"+file2.Name())
				auth.Stdin, _ = cat.StdoutPipe()
				auth.Stdout = os.Stdout
				auth.Stderr = os.Stderr
				_ = auth.Start()
				_ = cat.Run()
				_ = auth.Wait()
			}
		}

	}
	os.RemoveAll(update_dir)
	fmt.Println("Update metadata wiped, launching qTox")
	launchqtox := exec.Command("/usr/bin/open", "-b", "im.tox.qtox")
}
