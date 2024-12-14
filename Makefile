.PHONY: all clean build-cli build-gui run-cli run-gui

CLI_DIR = final-project-2
GUI_DIR = final-project-2-gui

all: build-cli build-gui

build-cli:
	$(MAKE) -C $(CLI_DIR)

build-gui:
	$(MAKE) -C $(GUI_DIR)

clean:
	$(MAKE) -C $(CLI_DIR) clean
	$(MAKE) -C $(GUI_DIR) clean

run-cli:
	cd $(CLI_DIR) && nohup ./final_project-2 > nohup.out 2>&1 & sleep 5

run-gui: run-cli
	cd $(GUI_DIR) && ./final-project-2-gui
