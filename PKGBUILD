# This is an example PKGBUILD file. Use this as a start to creating your own,
# and remove these comments. For more information, see 'man PKGBUILD'.
# NOTE: Please fill out the license field for your package! If it is unknown,
# then please put 'unknown'.

# Maintainer: Dasoftver LLC <vely@vely.dev>
pkgname=vely
pkgver=VV_VER
pkgrel=VV_REL
epoch=
pkgdesc="Development framework for C on Linux"
arch=("x86_64")
url="https://vely.dev"
license=('EPL-2.0')
groups=()
depends=(make gcc openssl curl 'mariadb-connector-c' fcgi 'postgresql-libs' sqlite3)
makedepends=(make gcc openssl curl 'mariadb-connector-c' fcgi 'postgresql-libs' sqlite3)
checkdepends=()
optdepends=()
provides=(vely)
conflicts=()
replaces=()
backup=()
options=()
install=
changelog=
source=("$pkgname-$pkgver.$pkgrel.tar.gz")
        
noextract=()
md5sums=(SKIP)
validpgpkeys=()

prepare() {
	cd "$pkgname-$pkgver.$pkgrel"
}

build() {
	cd "$pkgname-$pkgver.$pkgrel"
	make DESTDIR="$pkgdir/" clean
	make DESTDIR="$pkgdir/"
}

check() {
	cd "$pkgname-$pkgver.$pkgrel"
}

package() {
	cd "$pkgname-$pkgver.$pkgrel"
	make DESTDIR="$pkgdir/" install
}
