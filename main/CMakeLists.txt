idf_component_register(SRCS "igs_main.c"
							"igs_mqtt.c"
							"igs_ota.c"
							"igs_blufi_security.c"
							"igs_util.c"
                    INCLUDE_DIRS "."
					EMBED_TXTFILES ${project_dir}/server_certs/ca_cert.pem)