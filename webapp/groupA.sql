# ************************************************************
# Sequel Pro SQL dump
# Version 4096
#
# http://www.sequelpro.com/
# http://code.google.com/p/sequel-pro/
#
# Host: 127.0.0.1 (MySQL 5.5.33)
# Database: groupA
# Generation Time: 2014-06-20 09:36:49 +0000
# ************************************************************


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;


# Dump of table Directory
# ------------------------------------------------------------

DROP TABLE IF EXISTS `Directory`;

CREATE TABLE `Directory` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `url` varchar(255) NOT NULL DEFAULT '',
  `description` varchar(255) NOT NULL DEFAULT '',
  `name` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `url` (`url`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

LOCK TABLES `Directory` WRITE;
/*!40000 ALTER TABLE `Directory` DISABLE KEYS */;

INSERT INTO `Directory` (`id`, `url`, `description`, `name`)
VALUES
	(12,'http://localhost:8888/directories/fake_rd','','');

/*!40000 ALTER TABLE `Directory` ENABLE KEYS */;
UNLOCK TABLES;


# Dump of table Resource
# ------------------------------------------------------------

DROP TABLE IF EXISTS `Resource`;

CREATE TABLE `Resource` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL DEFAULT '',
  `description` varchar(255) NOT NULL DEFAULT '',
  `currentValue` decimal(10,3) NOT NULL DEFAULT '0.000',
  `resType` enum('sensor','switch','dimmer') NOT NULL DEFAULT 'sensor',
  `unit` varchar(32) NOT NULL DEFAULT '',
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `url` varchar(255) NOT NULL DEFAULT '',
  `directoryId` int(11) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `url` (`url`),
  KEY `directoryId` (`directoryId`),
  CONSTRAINT `resource_ibfk_1` FOREIGN KEY (`directoryId`) REFERENCES `Directory` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

LOCK TABLES `Resource` WRITE;
/*!40000 ALTER TABLE `Resource` DISABLE KEYS */;

INSERT INTO `Resource` (`id`, `name`, `description`, `currentValue`, `resType`, `unit`, `timestamp`, `url`, `directoryId`)
VALUES
	(23,'','',0.000,'switch','','2014-06-19 21:43:25','/fan',12),
	(25,'','',0.000,'sensor','m/s^2','2014-06-19 21:43:25','/accelerometer',12),
	(26,'','',0.000,'dimmer','lux','2014-06-19 21:55:10','/light',12);

/*!40000 ALTER TABLE `Resource` ENABLE KEYS */;
UNLOCK TABLES;


# Dump of table Sample
# ------------------------------------------------------------

DROP TABLE IF EXISTS `Sample`;

CREATE TABLE `Sample` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `resourceId` int(11) unsigned NOT NULL,
  `value` decimal(10,3) NOT NULL DEFAULT '0.000',
  `timeStamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `resourceId` (`resourceId`),
  CONSTRAINT `sample_ibfk_1` FOREIGN KEY (`resourceId`) REFERENCES `Resource` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

LOCK TABLES `Sample` WRITE;
/*!40000 ALTER TABLE `Sample` DISABLE KEYS */;

INSERT INTO `Sample` (`id`, `resourceId`, `value`, `timeStamp`)
VALUES
	(1,25,0.100,'2014-06-19 21:48:45'),
	(2,25,0.100,'2014-06-19 21:48:47'),
	(3,25,0.100,'2014-06-19 21:48:48'),
	(4,25,0.100,'2014-06-19 21:48:48'),
	(5,25,0.120,'2014-06-19 21:48:54');

/*!40000 ALTER TABLE `Sample` ENABLE KEYS */;
UNLOCK TABLES;



/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
