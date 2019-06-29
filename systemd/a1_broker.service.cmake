[Unit]
Description=Zepcam ipc init
#After=sysinit.target local-fs.target
#Before=basic.target

[Service]
Type=forking
PIDFile=/run/ipcinit.pid
ExecStart=/usr/bin/ipcinit
Restart=always

[Install]
WantedBy=multi-user.target
