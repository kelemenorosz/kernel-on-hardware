#!/bin/bash

getopts ":f:" USB_PATH

case $USB_PATH in

	("f") 	
		if [[ ! -d $OPTARG ]]; then
			echo "Removable media not attached."
			exit
		fi

		if [[ -f "${OPTARG}/STUB.ELF" ]]; then
			echo "STUB.ELF found on removable media; Removing STUB.ELF from removable media"
			rm "${OPTARG}/STUB.ELF"
		fi

		if [[ -f "${OPTARG}/KERNEL.ELF" ]]; then
			echo "KERNEL.ELF found on removable media; Removing KERNEL.ELF from removable media"
			rm "${OPTARG}/KERNEL.ELF"
		fi

		if [[ -f "./STUB.ELF" ]]; then
			echo "Copying STUB.ELF to removable media"
			cp STUB.ELF "${OPTARG}"		
		fi

		if [[ -f "./KERNEL.ELF" ]]; then
			echo "Copying KERNEL.ELF to removable media"
			cp KERNEL.ELF "${OPTARG}"		
		fi

	;;
	(":" | "?") 
		echo "Usage -f <USB_PATH>"
	;;

esac
