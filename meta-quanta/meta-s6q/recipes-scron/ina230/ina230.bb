SUMMARY = "Scron INA230 test"
PR = "r1"
LICENSE = "CLOSED"

S = "${WORKDIR}"

DEPENDS = " i2c-tools"

SRC_URI = "\
    file://main.c \
    file://main.h \
    file://ina230.c \
    file://ina230.h \
"

do_compile() {
    ${CC} main.c ina230.c -o scron-test -li2c -Wall
}
