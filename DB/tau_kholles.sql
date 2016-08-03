-- phpMyAdmin SQL Dump
-- version 4.1.14
-- http://www.phpmyadmin.net
--
-- Hoszt: 127.0.0.1
-- Létrehozás ideje: 2016. Júl 30. 12:19
-- Szerver verzió: 5.6.17
-- PHP verzió: 5.5.12

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Adatbázis: `lataupe`
--

-- --------------------------------------------------------

--
-- Tábla szerkezet ehhez a táblához `tau_kholles`
--

CREATE TABLE IF NOT EXISTS `tau_kholles` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `id_users` int(11) NOT NULL,
  `id_timeslots` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
