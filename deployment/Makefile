#  2023 eCTF
#  Deployment Makefile
#  Kyle Scaplen
#
#  (c) 2023 The MITRE Corporation
#
# This source file is part of an example system for MITRE's 2023 Embedded System CTF (eCTF).
# This code is being provided only for educational purposes for the 2023 MITRE eCTF competition,
# and may not meet MITRE standards for quality. Use this code at your own risk!

# function to check required arguments
check_defined = \
	$(strip $(foreach 1,$1, \
		$(call __check_defined,$1)))
__check_defined = \
	$(if $(value $1),, \
		$(error Undefined $1))


all:
	$(call check_defined SECRETS_DIR)
	gcc gen_key.c ./lib/libhydrogen/hydrogen.c -o /tmp/gen_key
	gcc gen_keypair.c ./lib/libhydrogen/hydrogen.c -o /tmp/gen_keypair
	/tmp/gen_key > ${SECRETS_DIR}/secret_key.txt
	/tmp/gen_keypair ${SECRETS_DIR}/signing_secret_key.txt ${SECRETS_DIR}/signing_public_key.txt
