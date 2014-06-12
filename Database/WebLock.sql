
USE `weblock` ;

-- -----------------------------------------------------
-- Table `mydb`.`users`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `weblock`.`users` (
  `iduser` INT NOT NULL AUTO_INCREMENT,
  `firstname` VARCHAR(45) NOT NULL,
  `lastname` VARCHAR(45) NOT NULL,
  `idkey` VARCHAR(45) NOT NULL,
  `permission` INT NOT NULL,
  PRIMARY KEY (`iduser`))
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`accesslog`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `weblock`.`accesslog` (
  `idaccesslog` INT NOT NULL AUTO_INCREMENT,
  `firstname` VARCHAR(45) NULL,
  `lastname` VARCHAR(45) NULL,
  `idkey` VARCHAR(45) NOT NULL,
  `date` DATETIME NOT NULL,
  `permission` INT NULL,
  PRIMARY KEY (`idaccesslog`))
ENGINE = InnoDB;

