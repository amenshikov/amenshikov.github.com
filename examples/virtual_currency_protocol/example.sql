CREATE TABLE `vc_payments` (
  `payment_id` int(11) NOT NULL AUTO_INCREMENT,
  `payment_invoice` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `payment_date` datetime NOT NULL,
  `payment_total` double NOT NULL,
  `payment_canceled` tinyint(4) NOT NULL DEFAULT '0',
  `payment_canceled_date` datetime NOT NULL,
  PRIMARY KEY (`payment_id`),
  KEY `payment_id` (`payment_id`),
  KEY `user_id` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

CREATE TABLE `vc_players` (
  `player_id` int(11) NOT NULL AUTO_INCREMENT,
  `player_username` varchar(255) NOT NULL,
  `player_email` varchar(255) NOT NULL,
  PRIMARY KEY (`player_id`),
  KEY `player_id` (`player_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;