/*
Navicat MySQL Data Transfer

Source Server         : 本机
Source Server Version : 50150
Source Host           : localhost:3306
Source Database       : ddb

Target Server Type    : MYSQL
Target Server Version : 50150
File Encoding         : 65001

Date: 2011-10-31 10:44:30
*/
DROP SCHEMA IF EXISTS DDB2011;

CREATE SCHEMA DDB2011;

USE DDB2011;

SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for `author`
-- ----------------------------
DROP TABLE IF EXISTS `author`;
CREATE TABLE `author` (
  `AID` char(10) NOT NULL DEFAULT '',
  `Name` char(20) DEFAULT NULL,
  PRIMARY KEY (`AID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of author
-- ----------------------------
INSERT INTO `Author` VALUES ('a1', 'Jim');
INSERT INTO `Author` VALUES ('a2', 'Robin');

-- ----------------------------
-- Table structure for `citation`
-- ----------------------------
DROP TABLE IF EXISTS `citation`;
CREATE TABLE `citation` (
  `Cite` char(10) DEFAULT NULL,
  `Cited` char(10) DEFAULT NULL,
  KEY `Citation_fk_1` (`Cite`),
  KEY `Citation_fk_2` (`Cited`),
  CONSTRAINT `Citation_fk_1` FOREIGN KEY (`Cite`) REFERENCES `paper` (`PID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `Citation_fk_2` FOREIGN KEY (`Cited`) REFERENCES `paper` (`PID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of citation
-- ----------------------------
INSERT INTO `Citation` VALUES ('t1', 't2');
INSERT INTO `Citation` VALUES ('t3', 't2');
INSERT INTO `Citation` VALUES ('t5', 't4');
INSERT INTO `Citation` VALUES ('t6', 't7');

-- ----------------------------
-- Table structure for `paper`
-- ----------------------------
DROP TABLE IF EXISTS `paper`;
CREATE TABLE `paper` (
  `PID` char(10) NOT NULL DEFAULT '',
  `Title` char(255) DEFAULT NULL,
  PRIMARY KEY (`PID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of paper
-- ----------------------------
INSERT INTO `Paper` VALUES ('t1', 'Keyword Search on RDBMS');
INSERT INTO `Paper` VALUES ('t2', 'Steiner Problem in DB');
INSERT INTO `Paper` VALUES ('t3', 'Efficient IR-Query over DB');
INSERT INTO `Paper` VALUES ('t4', 'Online Cluster Problems');
INSERT INTO `Paper` VALUES ('t5', 'Keyword Query over Web');
INSERT INTO `Paper` VALUES ('t6', 'Query Optimization on DB');
INSERT INTO `Paper` VALUES ('t7', 'Parameterized Complexity');

-- ----------------------------
-- Table structure for `paper-author`
-- ----------------------------
DROP TABLE IF EXISTS `paper-author`;
CREATE TABLE `paper-author` (
  `PID` char(10) DEFAULT NULL,
  `AID` char(10) DEFAULT NULL,
  KEY `Paper-Author_fk_1` (`PID`),
  KEY `Paper-Author_fk_2` (`AID`),
  CONSTRAINT `Paper-Author_fk_1` FOREIGN KEY (`PID`) REFERENCES `paper` (`PID`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `Paper-Author_fk_2` FOREIGN KEY (`AID`) REFERENCES `author` (`AID`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of paper-author
-- ----------------------------
INSERT INTO `Paper-Author` VALUES ('t2', 'a1');
INSERT INTO `Paper-Author` VALUES ('t4', 'a1');
INSERT INTO `Paper-Author` VALUES ('t3', 'a2');
INSERT INTO `Paper-Author` VALUES ('t4', 'a2');
INSERT INTO `Paper-Author` VALUES ('t5', 'a2');
INSERT INTO `Paper-Author` VALUES ('t6', 'a2');
INSERT INTO `Paper-Author` VALUES ('t7', 'a2');
