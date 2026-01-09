-- IDS projekt 2024/2025
-- Autori: Hugo Bohácsek (xbohach00), Filip Jenis (xjenisf00)

---- Odstránenie prípadných skôr vytvorených objektov ----
BEGIN
    EXECUTE IMMEDIATE 'DROP TABLE Prevod CASCADE CONSTRAINTS';
EXCEPTION
    WHEN OTHERS THEN NULL;
END;
/

BEGIN
    EXECUTE IMMEDIATE 'DROP TABLE Vyber CASCADE CONSTRAINTS';
EXCEPTION
    WHEN OTHERS THEN NULL;
END;
/

BEGIN
    EXECUTE IMMEDIATE 'DROP TABLE Vklad CASCADE CONSTRAINTS';
EXCEPTION
    WHEN OTHERS THEN NULL;
END;
/

BEGIN
    EXECUTE IMMEDIATE 'DROP TABLE Operacia CASCADE CONSTRAINTS';
EXCEPTION
    WHEN OTHERS THEN NULL;
END;
/

BEGIN
    EXECUTE IMMEDIATE 'DROP TABLE Disponent CASCADE CONSTRAINTS';
EXCEPTION
    WHEN OTHERS THEN NULL;
END;
/

BEGIN
    EXECUTE IMMEDIATE 'DROP TABLE Ucet CASCADE CONSTRAINTS';
EXCEPTION
    WHEN OTHERS THEN NULL;
END;
/

BEGIN
    EXECUTE IMMEDIATE 'DROP TABLE Klient CASCADE CONSTRAINTS';
EXCEPTION
    WHEN OTHERS THEN NULL;
END;
/

BEGIN
    EXECUTE IMMEDIATE 'DROP TABLE Zamestnanec CASCADE CONSTRAINTS';
EXCEPTION
    WHEN OTHERS THEN NULL;
END;
/

BEGIN
    EXECUTE IMMEDIATE 'DROP SEQUENCE idOperacieSeq';
EXCEPTION
    WHEN OTHERS THEN NULL;
END;
/

BEGIN
    EXECUTE IMMEDIATE 'DROP SEQUENCE zamestnanciSeq';
EXCEPTION
    WHEN OTHERS THEN NULL;
END;
/

BEGIN
    EXECUTE IMMEDIATE 'DROP SEQUENCE cisloUctuSeq';
EXCEPTION
    WHEN OTHERS THEN NULL;
END;
/

BEGIN
    EXECUTE IMMEDIATE 'DROP TRIGGER tr_kontrola_zostatku';
EXCEPTION
    WHEN OTHERS THEN NULL;
END;
/

BEGIN
    EXECUTE IMMEDIATE 'DROP TRIGGER tr_aktualizace_zustatku';
EXCEPTION
    WHEN OTHERS THEN NULL;
END;
/

BEGIN
    EXECUTE IMMEDIATE 'DROP PROCEDURE p_generuj_vypis';
EXCEPTION
    WHEN OTHERS THEN NULL;
END;
/

BEGIN
    EXECUTE IMMEDIATE 'DROP PROCEDURE p_prevod_mezi_ucty';
EXCEPTION
    WHEN OTHERS THEN NULL;
END;
/

BEGIN
    EXECUTE IMMEDIATE 'DROP MATERIALIZED VIEW mv_prehled_klientu';
EXCEPTION
    WHEN OTHERS THEN NULL;
END;
/

---- Vytvorenie sekvencií ----
CREATE SEQUENCE zamestnanciSeq
    START WITH 1
    INCREMENT BY 1;

CREATE SEQUENCE cisloUctuSeq
    START WITH 1000000000
    INCREMENT BY 1;

CREATE SEQUENCE idOperacieSeq
    START WITH 1
    INCREMENT BY 1;

---- Vytvorenie objektov ----
CREATE TABLE Klient (
    rodneCisloKlienta   VARCHAR(10)         NOT NULL    PRIMARY KEY,
    meno                NVARCHAR2(255)      NOT NULL,
    priezvisko          NVARCHAR2(255)      NOT NULL,
    ulica               NVARCHAR2(255)      NOT NULL,
    mesto               NVARCHAR2(255)      NOT NULL,
    psc                 VARCHAR(5)          NOT NULL,
    email               VARCHAR(320)        NOT NULL
                                            UNIQUE
                                            CHECK(REGEXP_LIKE(email, '[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}')),
    telefon             VARCHAR(20)         NOT NULL,

    -- Obmedzenie hodnôt - kontrola formátu rodného čísla
    CONSTRAINT rodneCislo_check CHECK (
        REGEXP_LIKE(rodneCisloKlienta, '^\d{10}$')
        AND (
            (
                REGEXP_LIKE(SUBSTR(rodneCisloKlienta, 3, 2), '^(0[1-9]|1[0-2])$')
                AND
                (
                    (
                        TO_NUMBER(SUBSTR(rodneCisloKlienta, 3, 2)) IN (1, 3, 5, 7, 8, 10, 12)
                        AND
                        REGEXP_LIKE(SUBSTR(rodneCisloKlienta, 5, 2), '^(0[1-9]|[1-2][0-9]|3[0-1])$')
                    )
                    OR
                    (
                        TO_NUMBER(SUBSTR(rodneCisloKlienta, 3, 2)) IN (4, 6, 9, 11)
                        AND
                        REGEXP_LIKE(SUBSTR(rodneCisloKlienta, 5, 2), '^(0[1-9]|[1-2][0-9]|30)$')
                    )
                    OR
                    (
                        TO_NUMBER(SUBSTR(rodneCisloKlienta, 3, 2)) = 2
                        AND
                        REGEXP_LIKE(SUBSTR(rodneCisloKlienta, 5, 2), '^(0[1-9]|1[0-9]|2[0-9])$')
                    )
                )
            )
            OR (
                REGEXP_LIKE(SUBSTR(rodneCisloKlienta, 3, 2), '^(5[1-9]|6[0-2])$')
                AND
                (
                    (
                        TO_NUMBER(SUBSTR(rodneCisloKlienta, 3, 2)) - 50 IN (1, 3, 5, 7, 8, 10, 12)
                            AND
                        REGEXP_LIKE(SUBSTR(rodneCisloKlienta, 5, 2), '^(0[1-9]|[1-2][0-9]|3[0-1])$')
                        )
                        OR
                    (
                        TO_NUMBER(SUBSTR(rodneCisloKlienta, 3, 2)) - 50 IN (4, 6, 9, 11)
                            AND
                        REGEXP_LIKE(SUBSTR(rodneCisloKlienta, 5, 2), '^(0[1-9]|[1-2][0-9]|30)$')
                        )
                        OR
                    (
                        TO_NUMBER(SUBSTR(rodneCisloKlienta, 3, 2)) - 50 = 2
                            AND
                        REGEXP_LIKE(SUBSTR(rodneCisloKlienta, 5, 2), '^(0[1-9]|1[0-9]|2[0-9])$')
                        )
                )
            )
        )
    )
);

CREATE TABLE Ucet (
    cisloUctu           NUMBER(20)      DEFAULT cisloUctuSeq.nextval    PRIMARY KEY,
    rodneCisloKlienta   VARCHAR(10)     NOT NULL,
    zostatok            DECIMAL(12,2)   DEFAULT 0,
    mena                VARCHAR(3)      NOT NULL,
    datumZalozenia      DATE            DEFAULT SYSDATE,

    CONSTRAINT FK_Ucet_Klient FOREIGN KEY (rodneCisloKlienta) REFERENCES Klient(rodneCisloKlienta) ON DELETE CASCADE
);

CREATE TABLE Disponent (
    rodneCisloDisponenta    VARCHAR(10)     NOT NULL,
    cisloUctu               NUMBER(20)      NOT NULL,
    limit                   NUMBER(12),
    datumPridania           DATE            DEFAULT SYSDATE,

    CONSTRAINT PK_Disponent PRIMARY KEY (rodneCisloDisponenta, cisloUctu),
    CONSTRAINT FK_Disponent_Klient FOREIGN KEY (rodneCisloDisponenta) REFERENCES Klient(rodneCisloKlienta) ON DELETE CASCADE,
    CONSTRAINT FK_Disponent_Ucet FOREIGN KEY (cisloUctu) REFERENCES Ucet(cisloUctu) ON DELETE CASCADE,
    CONSTRAINT limit_check CHECK (limit IS NULL OR limit > 0)
);

CREATE TABLE Zamestnanec (
    idZamestnanca   INTEGER         DEFAULT zamestnanciSeq.nextval  PRIMARY KEY,
    meno            NVARCHAR2(255)  NOT NULL,
    priezvisko      NVARCHAR2(255)  NOT NULL,
    pracovnaPozicia NVARCHAR2(128)  NOT NULL,
    datumNastupu    DATE            DEFAULT SYSDATE
);

-- Reprezentácia generalizácie/špecializácie pomocou tabuľky pre nadtyp a tabuliek pre podtypy s primárnym kľúčom nadtypu
-- Tento spôsob sme vybrali, pretože je najistesjší a keďže každá podtrieda má vlastné atribúty, ostatné triedy by mali veľa NULL hodnôt
CREATE TABLE Operacia (
    idOperacie      INTEGER         DEFAULT idOperacieSeq.nextval,
    zadavatel       VARCHAR(10)     NOT NULL,
    vykonavatel     INTEGER         NOT NULL,
    cisloUctu       NUMBER(20)      NOT NULL,
    datumCas        TIMESTAMP       DEFAULT CURRENT_TIMESTAMP,
    ciastka         DECIMAL(12,2)   NOT NULL,
    mena            VARCHAR(3)      NOT NULL,
    stav            NVARCHAR2(10)   NOT NULL,
    typ             NVARCHAR2(20)   NOT NULL,

    CONSTRAINT PK_Operacia PRIMARY KEY (idOperacie, cisloUctu),
    CONSTRAINT FK_Operacia_Klient FOREIGN KEY (zadavatel) REFERENCES Klient(rodneCisloKlienta),
    CONSTRAINT FK_Operacia_Zamestnanec FOREIGN KEY (vykonavatel) REFERENCES Zamestnanec(idZamestnanca),
    CONSTRAINT FK_Operacia_Ucet FOREIGN KEY (cisloUctu) REFERENCES Ucet(cisloUctu) ON DELETE CASCADE,
    CONSTRAINT stav_check CHECK (stav in ('VYKONANÁ', 'ZAMIETNUTÁ', 'ČAKAJÚCA')),
    CONSTRAINT typ_check CHECK (typ in ('VKLAD', 'VÝBER', 'PREVOD')),
    CONSTRAINT ciastka_check CHECK (ciastka > 0)
);

-- Podtrieda Operácie
CREATE TABLE Vklad (
    idOperacie  INTEGER,
    cisloUctu   NUMBER(20)      NOT NULL,
    miesto      NVARCHAR2(255)  NOT NULL,

    CONSTRAINT PK_Vklad PRIMARY KEY (idOperacie, cisloUctu),
    CONSTRAINT FK_Vklad_Operacia FOREIGN KEY (idOperacie, cisloUctu) REFERENCES Operacia(idOperacie, cisloUctu) ON DELETE CASCADE
);

-- Podtrieda Operácie
CREATE TABLE Vyber (
    idOperacie  INTEGER,
    cisloUctu   NUMBER(20)      NOT NULL,
    miesto      NVARCHAR2(255)  NOT NULL,

    CONSTRAINT PK_Vyber PRIMARY KEY (idOperacie, cisloUctu),
    CONSTRAINT FK_Vyber_ID FOREIGN KEY (idOperacie, cisloUctu) REFERENCES Operacia(idOperacie, cisloUctu) ON DELETE CASCADE
);

-- Podtrida Operácie
-- Reprezentácia generalizácie/špecializácie pomocou prístupu všetko v jednej tabuľke
-- Tento prístup sme vybrali, nakoľko interný prevod a prevod do inej banky sa líšia len v atribúte kodBanky - preto sme uznali za vhodné ich spojiť do jednej tabuľky
CREATE TABLE Prevod (
    idOperacie  INTEGER,
    cisloUctu   NUMBER(20)      NOT NULL,
    protiucet   VARCHAR(34)     NOT NULL,
    typPrevodu  NVARCHAR2(20)   NOT NULL,
    kodBanky    VARCHAR(10)      DEFAULT NULL,

    CONSTRAINT PK_Prevod PRIMARY KEY (idOperacie, cisloUctu),
    CONSTRAINT FK_Prevod_ID FOREIGN KEY (idOperacie, cisloUctu) REFERENCES Operacia(idOperacie, cisloUctu) ON DELETE CASCADE,
    CONSTRAINT typPrevodu_check CHECK (typPrevodu in ('INTERNÝ', 'EXTERNÝ')),
    CONSTRAINT typPrevodu_kodBanky_check CHECK(
        (
        typPrevodu = 'INTERNÝ'
        AND
        kodBanky IS NULL
        )
        OR
        (
        typPrevodu = 'EXTERNÝ'
        AND
        kodBanky IS NOT NULL
        )
    )
);

---- Vloženie vzorových dát ----

INSERT INTO Klient (rodneCisloKlienta, meno, priezvisko, ulica, mesto, psc, email, telefon)
VALUES ('7602151234', 'Jan', 'Novák', 'Hlavní 123', 'Praha', '12345', 'jan.novak@email.cz', '+420123456789');

INSERT INTO Klient (rodneCisloKlienta, meno, priezvisko, ulica, mesto, psc, email, telefon)
VALUES ('8556234321', 'Marie', 'Svobodová', 'Dlouhá 456', 'Brno', '61200', 'marie.s@email.cz', '+420987654321');

INSERT INTO Klient (rodneCisloKlienta, meno, priezvisko, ulica, mesto, psc, email, telefon)
VALUES ('9208305678', 'Petr', 'Dvořák', 'Krátká 789', 'Ostrava', '98765', 'petr.dvorak@email.cz', '+420567891234');

INSERT INTO Klient (rodneCisloKlienta, meno, priezvisko, ulica, mesto, psc, email, telefon)
VALUES ('7954178765', 'Eva', 'Nováková', 'Široká 321', 'Plzeň', '45678', 'eva.novakova@email.cz', '+420432198765');

INSERT INTO Klient (rodneCisloKlienta, meno, priezvisko, ulica, mesto, psc, email, telefon)
VALUES ('9103154567', 'Pavel', 'Malý', 'Polní 111', 'Olomouc', '77900', 'pavel.maly@email.cz', '+420776123456');

INSERT INTO Klient (rodneCisloKlienta, meno, priezvisko, ulica, mesto, psc, email, telefon)
VALUES ('7159108642', 'Anna', 'Veselá', 'Lipová 222', 'Liberec', '46001', 'anna.vesela@email.cz', '+420601234567');

INSERT INTO Zamestnanec (meno, priezvisko, pracovnaPozicia, datumNastupu)
VALUES ('Tomáš', 'Černý', 'Pokladník', TO_DATE('2021-01-15', 'YYYY-MM-DD'));

INSERT INTO Zamestnanec (meno, priezvisko, pracovnaPozicia, datumNastupu)
VALUES ('Jana', 'Veselá', 'Správce účtů', TO_DATE('2020-05-10', 'YYYY-MM-DD'));

INSERT INTO Zamestnanec (meno, priezvisko, pracovnaPozicia, datumNastupu)
VALUES ('Martin', 'Horák', 'Manažer pobočky', TO_DATE('2018-11-05', 'YYYY-MM-DD'));

INSERT INTO Zamestnanec (meno, priezvisko, pracovnaPozicia, datumNastupu)
VALUES ('Lucie', 'Procházková', 'Zákaznická podpora', TO_DATE('2022-03-20', 'YYYY-MM-DD'));

-- Účty - Vytvorenie účtov pre každého klienta
INSERT INTO Ucet (rodneCisloKlienta, zostatok, mena, datumZalozenia)
VALUES ('7602151234', 5000.00, 'EUR', TO_DATE('2020-01-15', 'YYYY-MM-DD'));

INSERT INTO Ucet (rodneCisloKlienta, zostatok, mena, datumZalozenia)
VALUES ('7602151234', 10000.00, 'CZK', TO_DATE('2021-03-22', 'YYYY-MM-DD'));

INSERT INTO Ucet (rodneCisloKlienta, zostatok, mena, datumZalozenia)
VALUES ('8556234321', 7500.50, 'EUR', TO_DATE('2019-11-30', 'YYYY-MM-DD'));

INSERT INTO Ucet (rodneCisloKlienta, zostatok, mena, datumZalozenia)
VALUES ('9208305678', 15000.00, 'EUR', TO_DATE('2022-05-10', 'YYYY-MM-DD'));

INSERT INTO Ucet (rodneCisloKlienta, zostatok, mena, datumZalozenia)
VALUES ('7954178765', 20000.00, 'USD', TO_DATE('2020-08-17', 'YYYY-MM-DD'));

INSERT INTO Ucet (rodneCisloKlienta, zostatok, mena, datumZalozenia)
VALUES ('9103154567', 8500.00, 'EUR', TO_DATE('2021-07-05', 'YYYY-MM-DD'));

INSERT INTO Ucet (rodneCisloKlienta, zostatok, mena, datumZalozenia)
VALUES ('7159108642', 12500.00, 'EUR', TO_DATE('2022-02-15', 'YYYY-MM-DD'));

INSERT INTO Ucet (rodneCisloKlienta, zostatok, mena, datumZalozenia)
VALUES ('9103154567', 3000.00, 'USD', TO_DATE('2021-09-18', 'YYYY-MM-DD'));

DECLARE
    acc1 VARCHAR2(20);
    acc2 VARCHAR2(20);
    acc3 VARCHAR2(20);
    acc4 VARCHAR2(20);
    acc5 VARCHAR2(20);
BEGIN
    SELECT cisloUctu INTO acc1 FROM ucet WHERE rodneCisloKlienta = '7602151234' AND mena = 'EUR' AND ROWNUM = 1;
    SELECT cisloUctu INTO acc2 FROM ucet WHERE rodneCisloKlienta = '7602151234' AND mena = 'CZK' AND ROWNUM = 1;
    SELECT cisloUctu INTO acc3 FROM ucet WHERE rodneCisloKlienta = '8556234321' AND ROWNUM = 1;
    SELECT cisloUctu INTO acc4 FROM ucet WHERE rodneCisloKlienta = '9208305678' AND ROWNUM = 1;
    SELECT cisloUctu INTO acc5 FROM ucet WHERE rodneCisloKlienta = '7954178765' AND ROWNUM = 1;

    INSERT INTO Disponent (rodneCisloDisponenta, cisloUctu, limit, datumPridania)
    VALUES ('8556234321', acc1, 1000.00, TO_DATE('2021-05-10', 'YYYY-MM-DD'));

    INSERT INTO Disponent (rodneCisloDisponenta, cisloUctu, limit, datumPridania)
    VALUES ('9208305678', acc2, NULL, TO_DATE('2022-01-15', 'YYYY-MM-DD'));

    INSERT INTO Disponent (rodneCisloDisponenta, cisloUctu, limit, datumPridania)
    VALUES ('7602151234', acc3, 2000.00, TO_DATE('2020-12-03', 'YYYY-MM-DD'));

    DECLARE
        emp1_id NUMBER;
        emp2_id NUMBER;
        emp3_id NUMBER;
        emp4_id NUMBER;
        op_id NUMBER;
    BEGIN
        SELECT idZamestnanca INTO emp1_id FROM zamestnanec WHERE meno = 'Tomáš' AND priezvisko = 'Černý' AND ROWNUM = 1;
        SELECT idZamestnanca INTO emp2_id FROM zamestnanec WHERE meno = 'Jana' AND priezvisko = 'Veselá' AND ROWNUM = 1;
        SELECT idZamestnanca INTO emp3_id FROM zamestnanec WHERE meno = 'Martin' AND priezvisko = 'Horák' AND ROWNUM = 1;
        SELECT idZamestnanca INTO emp4_id FROM zamestnanec WHERE meno = 'Lucie' AND priezvisko = 'Procházková' AND ROWNUM = 1;

        -- Operácia vkladu
        INSERT INTO Operacia (cisloUctu, zadavatel, vykonavatel, datumCas, ciastka, mena, stav, typ)
        VALUES (acc1, '7602151234', emp1_id, TIMESTAMP '2023-01-15 10:30:00', 1000.00, 'EUR', 'VYKONANÁ', 'VKLAD');

        SELECT MAX(idOperacie) INTO op_id FROM Operacia WHERE cisloUctu = acc1 AND zadavatel = '7602151234';

        INSERT INTO Vklad (idOperacie, cisloUctu, miesto)
        VALUES (op_id, acc1,'Pobočka Praha Hlavní');

        -- Operácia výberu
        INSERT INTO Operacia (cisloUctu, zadavatel, vykonavatel, datumCas, ciastka, mena, stav, typ)
        VALUES (acc3, '8556234321', emp2_id, TIMESTAMP '2023-02-20 14:45:00', 500.00, 'EUR', 'VYKONANÁ', 'VÝBER');

        SELECT MAX(idOperacie) INTO op_id FROM Operacia WHERE cisloUctu = acc3 AND zadavatel = '8556234321';

        INSERT INTO Vyber (idOperacie, cisloUctu, miesto)
        VALUES (op_id, acc3, 'Bankomat Brno Centrum');

        -- Operácia interného prevodu
        INSERT INTO Operacia (cisloUctu, zadavatel, vykonavatel, datumCas, ciastka, mena, stav, typ)
        VALUES (acc1, '7602151234', emp3_id, TIMESTAMP '2023-03-10 09:15:00', 750.00, 'EUR', 'VYKONANÁ', 'PREVOD');

        SELECT MAX(idOperacie) INTO op_id FROM Operacia WHERE cisloUctu = acc1 AND zadavatel = '7602151234' AND ciastka = 750.00;

        INSERT INTO Prevod (idOperacie, cisloUctu, protiucet, typPrevodu)
        VALUES (op_id, acc1, acc4, 'INTERNÝ');

        INSERT INTO Operacia (cisloUctu, zadavatel, vykonavatel, datumCas, ciastka, mena, stav, typ)
        VALUES (acc5, '7954178765', emp4_id, TIMESTAMP '2023-03-15 16:30:00', 1000.00, 'USD', 'VYKONANÁ', 'PREVOD');

        SELECT MAX(idOperacie) INTO op_id FROM Operacia WHERE cisloUctu = acc5 AND zadavatel = '7954178765';

        INSERT INTO Prevod (idOperacie, cisloUctu, protiucet, typPrevodu, kodBanky)
        VALUES (op_id, acc5, 'GB29NWBK60161331926819', 'EXTERNÝ', 'NWBKGB2L');

        INSERT INTO Operacia (cisloUctu, zadavatel, vykonavatel, datumCas, ciastka, mena, stav, typ)
        VALUES (acc1, '7602151234', emp1_id, TIMESTAMP '2023-04-05 11:20:00', 50000.00, 'EUR', 'ZAMIETNUTÁ', 'VÝBER');

        SELECT MAX(idOperacie) INTO op_id FROM Operacia WHERE cisloUctu = acc1 AND zadavatel = '7602151234' AND ciastka = 50000.00;

        INSERT INTO Vyber (idOperacie, cisloUctu, miesto)
        VALUES (op_id, acc1, 'Pobočka Praha Hlavní');

        INSERT INTO Operacia (cisloUctu, zadavatel, vykonavatel, datumCas, ciastka, mena, stav, typ)
        VALUES (acc5, '7954178765', emp4_id, TIMESTAMP '2023-04-20 13:45:00', 5000.00, 'USD', 'ČAKAJÚCA', 'PREVOD');

        SELECT MAX(idOperacie) INTO op_id FROM Operacia WHERE cisloUctu = acc5 AND zadavatel = '7954178765' AND ciastka = 5000.00;

        -- Vloženie detailov prevodu
        INSERT INTO Prevod (idOperacie, cisloUctu, protiucet, typPrevodu, kodBanky)
        VALUES (op_id, acc5, 'DE89370400440532013000', 'EXTERNÝ', 'DEUTDEFF');

        INSERT INTO Operacia (cisloUctu, zadavatel, vykonavatel, datumCas, ciastka, mena, stav, typ)
        VALUES (acc1, '7602151234', emp1_id, TIMESTAMP '2023-05-12 09:30:00', 200.00, 'EUR', 'VYKONANÁ', 'VÝBER');

        SELECT MAX(idOperacie) INTO op_id FROM Operacia WHERE cisloUctu = acc1 AND zadavatel = '7602151234' AND ciastka = 200.00;

        INSERT INTO Vyber (idOperacie, cisloUctu, miesto)
        VALUES (op_id, acc1, 'Bankomat Praha Centrum');

        INSERT INTO Operacia (cisloUctu, zadavatel, vykonavatel, datumCas, ciastka, mena, stav, typ)
        VALUES (acc3, '8556234321', emp2_id, TIMESTAMP '2023-06-05 14:15:00', 1500.00, 'EUR', 'VYKONANÁ', 'VKLAD');

        SELECT MAX(idOperacie) INTO op_id FROM Operacia WHERE cisloUctu = acc3 AND zadavatel = '8556234321' AND ciastka = 1500.00;

        INSERT INTO Vklad (idOperacie, cisloUctu, miesto)
        VALUES (op_id, acc3, 'Pobočka Brno Hlavní');

        INSERT INTO Operacia (cisloUctu, zadavatel, vykonavatel, datumCas, ciastka, mena, stav, typ)
        VALUES (acc4, '9208305678', emp3_id, TIMESTAMP '2023-07-20 16:45:00', 2000.00, 'EUR', 'VYKONANÁ', 'PREVOD');

        SELECT MAX(idOperacie) INTO op_id FROM Operacia WHERE cisloUctu = acc4 AND zadavatel = '9208305678' AND ciastka = 2000.00;

        INSERT INTO Prevod (idOperacie, cisloUctu, protiucet, typPrevodu)
        VALUES (op_id, acc4, acc1, 'INTERNÝ');
    END;
END;
/

-- *** 1. DATABÁZOVÉ TRIGGERY *** --
-- 1.1 Trigger ktorý kontroluje zostatok pred výberom / prevodom
CREATE OR REPLACE TRIGGER tr_kontrola_zostatku
BEFORE INSERT ON Operacia
FOR EACH ROW
DECLARE
    v_zostatok Ucet.zostatok%TYPE;
    v_mena Ucet.mena%TYPE;
    v_limit Disponent.limit%TYPE := NULL;
    v_vlastnik VARCHAR(10);
    v_nedostatocny_zostatok EXCEPTION;
    v_rozna_mena EXCEPTION;
    v_prekroceny_limit EXCEPTION;
BEGIN
    -- Kontrola pre výbery a prevody v stave ČAKAJÚCA
    IF (:NEW.typ IN ('VÝBER', 'PREVOD') AND :NEW.stav = 'ČAKAJÚCA') THEN
        SELECT zostatok, mena, rodneCisloKlienta 
        INTO v_zostatok, v_mena, v_vlastnik
        FROM Ucet 
        WHERE cisloUctu = :NEW.cisloUctu;
        
        IF (:NEW.mena != v_mena) THEN
            RAISE v_rozna_mena;
        END IF;
        
        IF (v_zostatok < :NEW.ciastka) THEN
            RAISE v_nedostatocny_zostatok;
        END IF;
        
        IF (:NEW.zadavatel != v_vlastnik) THEN
            BEGIN
                SELECT limit INTO v_limit
                FROM Disponent
                WHERE rodneCisloDisponenta = :NEW.zadavatel
                AND cisloUctu = :NEW.cisloUctu;
                
                IF (v_limit IS NOT NULL AND :NEW.ciastka > v_limit) THEN
                    RAISE v_prekroceny_limit;
                END IF;
            EXCEPTION
                WHEN NO_DATA_FOUND THEN
                    :NEW.stav := 'ZAMIETNUTÁ';
            END;
        END IF;
    END IF;
EXCEPTION
    WHEN v_nedostatocny_zostatok THEN
        :NEW.stav := 'ZAMIETNUTÁ';
    WHEN v_rozna_mena THEN
        :NEW.stav := 'ZAMIETNUTÁ';
    WHEN v_prekroceny_limit THEN
        :NEW.stav := 'ZAMIETNUTÁ';
END;
/

-- 1.2 Trigger pre aktualizovanie zostatku po operácii
CREATE OR REPLACE TRIGGER tr_aktualizace_zustatku
AFTER INSERT OR UPDATE OF stav ON Operacia
FOR EACH ROW
DECLARE
    v_protiucet Prevod.protiucet%TYPE;
    v_typ_prevodu Prevod.typPrevodu%TYPE;
BEGIN
    -- Aktualizácia zostatku len keď sa stav zmení na VYKONANÁ
    IF (:NEW.stav = 'VYKONANÁ') THEN
        IF (:NEW.typ = 'VKLAD') THEN
            UPDATE Ucet
            SET zostatok = zostatok + :NEW.ciastka
            WHERE cisloUctu = :NEW.cisloUctu;
        
        ELSIF (:NEW.typ = 'VÝBER') THEN
            UPDATE Ucet
            SET zostatok = zostatok - :NEW.ciastka
            WHERE cisloUctu = :NEW.cisloUctu;
        
        ELSIF (:NEW.typ = 'PREVOD') THEN
            UPDATE Ucet
            SET zostatok = zostatok - :NEW.ciastka
            WHERE cisloUctu = :NEW.cisloUctu;
            
            BEGIN
                SELECT protiucet, typPrevodu 
                INTO v_protiucet, v_typ_prevodu
                FROM Prevod
                WHERE idOperacie = :NEW.idOperacie AND cisloUctu = :NEW.cisloUctu;
                
                IF (v_typ_prevodu = 'INTERNÝ') THEN
                    UPDATE Ucet
                    SET zostatok = zostatok + :NEW.ciastka
                    WHERE cisloUctu = v_protiucet;
                END IF;
            EXCEPTION
                WHEN NO_DATA_FOUND THEN
                    NULL;
            END;
        END IF;
    END IF;
END;
/

-- *** 2. PROCEDÚRY *** --

-- 2.1 Procedúra pre vygenerovanie výpisu transakcí klienta za určité obdobie
CREATE OR REPLACE PROCEDURE p_generuj_vypis(
    p_rodne_cislo IN Klient.rodneCisloKlienta%TYPE,
    p_od_datum IN DATE,
    p_do_datum IN DATE
) IS
    v_klient_exist NUMBER;
    v_klient_info Klient%ROWTYPE;
    v_ucet Ucet%ROWTYPE;
    
    CURSOR c_ucty IS
        SELECT *
        FROM Ucet
        WHERE rodneCisloKlienta = p_rodne_cislo;
    
    CURSOR c_operacie(p_cislo_uctu IN Ucet.cisloUctu%TYPE) IS
        SELECT o.*, 
               CASE 
                   WHEN o.typ = n'VKLAD' THEN n'VKLAD'
                   WHEN o.typ = n'VÝBER' THEN n'VÝBER'
                   WHEN o.typ = n'PREVOD' THEN
                        CASE
                            WHEN (SELECT typPrevodu FROM Prevod WHERE idOperacie = o.idOperacie AND cisloUctu = o.cisloUctu) = n'INTERNÝ'
                            THEN n'INTERNÍ PREVOD'
                            ELSE n'EXTERNÍ PREVOD'
                        END
               END AS detailny_typ,
               CASE
                   WHEN o.typ = n'VKLAD' THEN (SELECT miesto FROM Vklad WHERE idOperacie = o.idOperacie AND cisloUctu = o.cisloUctu)
                   WHEN o.typ = n'VÝBER' THEN (SELECT miesto FROM Vyber WHERE idOperacie = o.idOperacie AND cisloUctu = o.cisloUctu)
                   WHEN o.typ = n'PREVOD' THEN
                        CASE
                            WHEN (SELECT typPrevodu FROM Prevod WHERE idOperacie = o.idOperacie AND cisloUctu = o.cisloUctu) = n'INTERNÝ'
                            THEN n'Účet: ' || (SELECT protiucet FROM Prevod WHERE idOperacie = o.idOperacie AND cisloUctu = o.cisloUctu)
                            ELSE n'IBAN: ' || (SELECT protiucet FROM Prevod WHERE idOperacie = o.idOperacie AND cisloUctu = o.cisloUctu) ||
                                 n', Kód banky: ' || (SELECT kodBanky FROM Prevod WHERE idOperacie = o.idOperacie AND cisloUctu = o.cisloUctu)
                        END
               END AS detail,
               z.meno || ' ' || z.priezvisko AS zamestnanec
        FROM Operacia o
        JOIN Zamestnanec z ON o.vykonavatel = z.idZamestnanca
        WHERE o.cisloUctu = p_cislo_uctu
          AND o.datumCas BETWEEN p_od_datum AND p_do_datum
        ORDER BY o.datumCas;
    
    v_pocet_operacii NUMBER;
    v_neplatny_klient EXCEPTION;
BEGIN
    SELECT COUNT(*) INTO v_klient_exist
    FROM Klient
    WHERE rodneCisloKlienta = p_rodne_cislo;
    
    IF v_klient_exist = 0 THEN
        RAISE v_neplatny_klient;
    END IF;
    
    SELECT * INTO v_klient_info
    FROM Klient
    WHERE rodneCisloKlienta = p_rodne_cislo;
    
    -- Výpis hlavičky
    DBMS_OUTPUT.PUT_LINE('=====================================================');
    DBMS_OUTPUT.PUT_LINE('VÝPIS TRANSAKCÍ KLIENTA');
    DBMS_OUTPUT.PUT_LINE('=====================================================');
    DBMS_OUTPUT.PUT_LINE('Klient: ' || v_klient_info.meno || ' ' || v_klient_info.priezvisko);
    DBMS_OUTPUT.PUT_LINE('Rodné číslo: ' || v_klient_info.rodneCisloKlienta);
    DBMS_OUTPUT.PUT_LINE('Adresa: ' || v_klient_info.ulica || ', ' || v_klient_info.mesto || ', ' || v_klient_info.psc);
    DBMS_OUTPUT.PUT_LINE('Obdobie: ' || TO_CHAR(p_od_datum, 'DD.MM.YYYY') || ' - ' || TO_CHAR(p_do_datum, 'DD.MM.YYYY'));
    DBMS_OUTPUT.PUT_LINE('=====================================================');
    
    -- Prechádzanie účtov klienta
    FOR r_ucet IN c_ucty LOOP
        v_ucet := r_ucet;
        DBMS_OUTPUT.PUT_LINE('');
        DBMS_OUTPUT.PUT_LINE('ÚČET: ' || v_ucet.cisloUctu || ' (' || v_ucet.mena || ')');
        DBMS_OUTPUT.PUT_LINE('Aktuálny zostatok: ' || v_ucet.zostatok || ' ' || v_ucet.mena);
        DBMS_OUTPUT.PUT_LINE('-----------------------------------------------------');
        DBMS_OUTPUT.PUT_LINE('DATUM A ČAS      | TYP        | ČIASTKA   | STAV      | DETAIL                         | PRACOVNÍK');
        DBMS_OUTPUT.PUT_LINE('-----------------------------------------------------');
        
        v_pocet_operacii := 0;
        
        -- Prechádzanie operácií na účte
        FOR r_operacia IN c_operacie(v_ucet.cisloUctu) LOOP
            v_pocet_operacii := v_pocet_operacii + 1;
            
            DBMS_OUTPUT.PUT_LINE(
                RPAD(TO_CHAR(r_operacia.datumCas, 'DD.MM.YYYY HH24:MI'), 17) || ' | ' ||
                RPAD(r_operacia.detailny_typ, 11) || ' | ' ||
                LPAD(r_operacia.ciastka || ' ' || r_operacia.mena, 9) || ' | ' ||
                RPAD(r_operacia.stav, 10) || ' | ' ||
                RPAD(SUBSTR(r_operacia.detail, 1, 30), 30) || ' | ' ||
                r_operacia.zamestnanec
            );
        END LOOP;
        
        IF v_pocet_operacii = 0 THEN
            DBMS_OUTPUT.PUT_LINE('Žiadne transakcie v danom období.');
        END IF;
        DBMS_OUTPUT.PUT_LINE('-----------------------------------------------------');
    END LOOP;
    
EXCEPTION
    WHEN v_neplatny_klient THEN
        DBMS_OUTPUT.PUT_LINE('Chyba: Klient s rodným číslom ' || p_rodne_cislo || ' neexistuje.');
    WHEN OTHERS THEN
        DBMS_OUTPUT.PUT_LINE('Došlo k chybe: ' || SQLERRM);
END p_generuj_vypis;
/

-- 2.2 Procedúra pre prevod medzi účtami
CREATE OR REPLACE PROCEDURE p_prevod_mezi_ucty(
    p_z_uctu IN Ucet.cisloUctu%TYPE,
    p_na_ucet IN VARCHAR2,
    p_suma IN Operacia.ciastka%TYPE,
    p_zadavatel IN Klient.rodneCisloKlienta%TYPE,
    p_vykonavatel IN Zamestnanec.idZamestnanca%TYPE,
    p_kod_banky IN Prevod.kodBanky%TYPE DEFAULT NULL,
    p_id_operace OUT Operacia.idOperacie%TYPE
) IS
    v_zostatok Ucet.zostatok%TYPE;
    v_mena Ucet.mena%TYPE;
    v_vlastnik Ucet.rodneCisloKlienta%TYPE;
    v_limit Disponent.limit%TYPE;
    v_je_disponent NUMBER;
    v_je_interny NUMBER := 0;
    v_typ_prevodu Prevod.typPrevodu%TYPE;
    v_stav Operacia.stav%TYPE := 'ČAKAJÚCA';
    
    e_nedostatocny_zostatok EXCEPTION;
    e_prekroceny_limit EXCEPTION;
    e_neopravneny_pristup EXCEPTION;
    e_neexistujici_ucet EXCEPTION;
    e_neplatna_hodnota EXCEPTION;
BEGIN
    IF p_suma <= 0 THEN
        RAISE e_neplatna_hodnota;
    END IF;
    
    BEGIN
        SELECT zostatok, mena, rodneCisloKlienta
        INTO v_zostatok, v_mena, v_vlastnik
        FROM Ucet
        WHERE cisloUctu = p_z_uctu;
    EXCEPTION
        WHEN NO_DATA_FOUND THEN
            RAISE e_neexistujici_ucet;
    END;
    
    IF v_zostatok < p_suma THEN
        RAISE e_nedostatocny_zostatok;
    END IF;
    
    IF v_vlastnik != p_zadavatel THEN
        SELECT COUNT(*) INTO v_je_disponent
        FROM Disponent
        WHERE rodneCisloDisponenta = p_zadavatel
          AND cisloUctu = p_z_uctu;
        
        IF v_je_disponent = 0 THEN
            RAISE e_neopravneny_pristup;
        END IF;
        
        BEGIN
            SELECT limit INTO v_limit
            FROM Disponent
            WHERE rodneCisloDisponenta = p_zadavatel
              AND cisloUctu = p_z_uctu;
            
            IF v_limit IS NOT NULL AND p_suma > v_limit THEN
                RAISE e_prekroceny_limit;
            END IF;
        EXCEPTION
            WHEN NO_DATA_FOUND THEN
                RAISE e_neopravneny_pristup;
        END;
    END IF;
    
    IF p_kod_banky IS NULL THEN
        BEGIN
            SELECT COUNT(*) INTO v_je_interny
            FROM Ucet
            WHERE cisloUctu = p_na_ucet;
            
            IF v_je_interny > 0 THEN
                v_typ_prevodu := 'INTERNÝ';
            ELSE
                RAISE e_neexistujici_ucet;
            END IF;
        EXCEPTION
            WHEN OTHERS THEN
                RAISE e_neexistujici_ucet;
        END;
    ELSE
        v_typ_prevodu := 'EXTERNÝ';
    END IF;
    
    INSERT INTO Operacia (
        zadavatel, vykonavatel, cisloUctu, datumCas, ciastka, mena, stav, typ
    ) VALUES (
        p_zadavatel, p_vykonavatel, p_z_uctu, CURRENT_TIMESTAMP, p_suma, v_mena, v_stav, 'PREVOD'
    ) RETURNING idOperacie INTO p_id_operace;
    
    INSERT INTO Prevod (
        idOperacie, cisloUctu, protiucet, typPrevodu, kodBanky
    ) VALUES (
        p_id_operace, p_z_uctu, p_na_ucet, v_typ_prevodu, p_kod_banky
    );
    
    UPDATE Operacia
    SET stav = 'VYKONANÁ'
    WHERE idOperacie = p_id_operace;
    
    COMMIT;
    
EXCEPTION
    WHEN e_nedostatocny_zostatok THEN
        RAISE_APPLICATION_ERROR(-20001, 'Nedostatočný zostatok na účte.');
    WHEN e_prekroceny_limit THEN
        RAISE_APPLICATION_ERROR(-20002, 'Čiastka prevodu prekračuje limit disponenta.');
    WHEN e_neopravneny_pristup THEN
        RAISE_APPLICATION_ERROR(-20003, 'Neoprávnený prístup k účtu.');
    WHEN e_neexistujici_ucet THEN
        RAISE_APPLICATION_ERROR(-20004, 'Účet neexistuje.');
    WHEN e_neplatna_hodnota THEN
        RAISE_APPLICATION_ERROR(-20005, 'Neplatná hodnota čiastky.');
    WHEN OTHERS THEN
        ROLLBACK;
        RAISE_APPLICATION_ERROR(-20099, 'Došlo k chybě: ' || SQLERRM);
END p_prevod_mezi_ucty;
/

-- *** 3. INDEXY *** --

EXPLAIN PLAN FOR
SELECT 
    K.mesto,
    O.typ,
    COUNT(*) AS pocet_operaci,
    SUM(O.ciastka) AS celkova_suma
FROM 
    Klient K
    JOIN Ucet U ON K.rodneCisloKlienta = U.rodneCisloKlienta
    JOIN Operacia O ON U.cisloUctu = O.cisloUctu
WHERE 
    O.stav = 'VYKONANÁ'
    AND O.datumCas BETWEEN TO_DATE('2023-01-01', 'YYYY-MM-DD') AND TO_DATE('2023-12-31', 'YYYY-MM-DD')
GROUP BY 
    K.mesto, O.typ
ORDER BY 
    K.mesto, O.typ;

SELECT PLAN_TABLE_OUTPUT FROM TABLE(DBMS_XPLAN.DISPLAY());

CREATE INDEX idx_operacia_datum_stav ON Operacia(datumCas, stav);

EXPLAIN PLAN FOR
SELECT 
    K.mesto,
    O.typ,
    COUNT(*) AS pocet_operaci,
    SUM(O.ciastka) AS celkova_suma
FROM 
    Klient K
    JOIN Ucet U ON K.rodneCisloKlienta = U.rodneCisloKlienta
    JOIN Operacia O ON U.cisloUctu = O.cisloUctu
WHERE 
    O.stav = 'VYKONANÁ'
    AND O.datumCas BETWEEN TO_DATE('2023-01-01', 'YYYY-MM-DD') AND TO_DATE('2023-12-31', 'YYYY-MM-DD')
GROUP BY 
    K.mesto, O.typ
ORDER BY 
    K.mesto, O.typ;

SELECT PLAN_TABLE_OUTPUT FROM TABLE(DBMS_XPLAN.DISPLAY());

-- *** 4. PRÍSTUPOVÉ PRÁVA *** --

/*
GRANT ALL ON Klient TO xjenisf00;
GRANT ALL ON Ucet TO xjenisf00;
GRANT ALL ON Operacia TO xjenisf00;
GRANT ALL ON Zamestnanec TO xjenisf00;
GRANT ALL ON Vklad TO xjenisf00;
GRANT ALL ON Vyber TO xjenisf00;
GRANT ALL ON Prevod TO xjenisf00;
GRANT ALL ON Disponent TO xjenisf00;

GRANT EXECUTE ON p_generuj_vypis TO xjenisf00;
GRANT EXECUTE ON p_prevod_mezi_ucty TO xjenisf00;
*/

-- *** 5. MATERIALIZOVANÝ POHĽAD *** --

CREATE MATERIALIZED VIEW mv_prehled_klientu
REFRESH COMPLETE ON DEMAND
AS
SELECT 
    K.rodneCisloKlienta,
    K.meno || ' ' || K.priezvisko AS meno_priezvisko,
    K.mesto,
    COUNT(DISTINCT U.cisloUctu) AS pocet_uctov,
    SUM(U.zostatok) AS celkovy_zostatok,
    MIN(U.mena) AS hlavna_mena,
    COUNT(DISTINCT D.rodneCisloDisponenta) AS pocet_disponentov,
    COUNT(DISTINCT O.idOperacie) AS pocet_operacii
FROM 
    Klient K
    LEFT JOIN Ucet U ON K.rodneCisloKlienta = U.rodneCisloKlienta
    LEFT JOIN Disponent D ON U.cisloUctu = D.cisloUctu
    LEFT JOIN Operacia O ON U.cisloUctu = O.cisloUctu AND O.stav = 'VYKONANÁ'
GROUP BY 
    K.rodneCisloKlienta, K.meno, K.priezvisko, K.mesto;

BEGIN
    DBMS_SNAPSHOT.REFRESH('mv_prehled_klientu');
END;
/

SELECT * FROM mv_prehled_klientu;

-- Pridelenie práv
GRANT ALL ON mv_prehled_klientu TO xjenisf00;

-- Druhý používateľ potom používa
-- SELECT * FROM xbohach00.mv_prehled_klientu

-- *** 6. KOMPLEXNÝ DOTAZ S WITH A CASE *** --

WITH 
    operace_souhrn AS (
        SELECT 
            K.mesto,
            O.typ,
            COUNT(*) AS pocet_operaci,
            SUM(O.ciastka) AS celkova_suma,
            AVG(O.ciastka) AS prumerna_castka
        FROM 
            Klient K
            JOIN Ucet U ON K.rodneCisloKlienta = U.rodneCisloKlienta
            JOIN Operacia O ON U.cisloUctu = O.cisloUctu
        WHERE 
            O.stav = 'VYKONANÁ'
            AND O.datumCas BETWEEN TO_DATE('2023-01-01', 'YYYY-MM-DD') AND TO_DATE('2023-12-31', 'YYYY-MM-DD')
        GROUP BY 
            K.mesto, O.typ
    ),
    klient_aktivity AS (
        SELECT 
            K.rodneCisloKlienta,
            K.meno || ' ' || K.priezvisko AS jmeno,
            K.mesto,
            COUNT(O.idOperacie) AS pocet_operaci,
            SUM(O.ciastka) AS celkova_suma,
            CASE 
                WHEN COUNT(O.idOperacie) > 5 THEN 'Veľmi aktívny'
                WHEN COUNT(O.idOperacie) > 2 THEN 'Aktívny'
                WHEN COUNT(O.idOperacie) > 0 THEN 'Málo aktívny'
                ELSE 'Neaktívny'
            END AS aktivita_kategorie,
            CASE 
                WHEN SUM(O.ciastka) > 10000 THEN 'Vysoký objem'
                WHEN SUM(O.ciastka) > 5000 THEN 'Stredný objem'
                WHEN SUM(O.ciastka) > 0 THEN 'Nízky objem'
                ELSE 'Žiadny objem'
            END AS objem_kategorie
        FROM 
            Klient K
            LEFT JOIN Ucet U ON K.rodneCisloKlienta = U.rodneCisloKlienta
            LEFT JOIN Operacia O ON U.cisloUctu = O.cisloUctu 
                AND O.stav = 'VYKONANÁ'
                AND O.datumCas BETWEEN TO_DATE('2023-01-01', 'YYYY-MM-DD') AND TO_DATE('2023-12-31', 'YYYY-MM-DD')
        GROUP BY 
            K.rodneCisloKlienta, K.meno, K.priezvisko, K.mesto
    )
SELECT 
    os.mesto,
    os.typ,
    os.pocet_operaci,
    os.celkova_suma,
    os.prumerna_castka,
    (SELECT COUNT(*) FROM klient_aktivity ka WHERE ka.mesto = os.mesto AND ka.aktivita_kategorie = 'Veľmi aktívny') AS pocet_velmi_aktivnich,
    (SELECT COUNT(*) FROM klient_aktivity ka WHERE ka.mesto = os.mesto AND ka.objem_kategorie = 'Vysoký objem') AS pocet_vysoky_objem
FROM 
    operace_souhrn os
ORDER BY 
    os.mesto, os.pocet_operaci DESC;

-- *** 7. DEMONŠTRÁCIA POUŽITIA TRIGGEROV A PROCEDÚR *** --

DECLARE
    v_ucet_id Ucet.cisloUctu%TYPE;
    v_zamestnanec_id Zamestnanec.idZamestnanca%TYPE;
    v_zostatok Ucet.zostatok%TYPE;
    v_rodne_cislo Klient.rodneCisloKlienta%TYPE;
    v_operace_id Operacia.idOperacie%TYPE;
BEGIN
    SELECT cisloUctu, zostatok, rodneCisloKlienta INTO v_ucet_id, v_zostatok, v_rodne_cislo
    FROM Ucet
    WHERE ROWNUM = 1;
    
    SELECT idZamestnanca INTO v_zamestnanec_id
    FROM Zamestnanec
    WHERE ROWNUM = 1;
    
    DBMS_OUTPUT.PUT_LINE('DEMONŠTRÁCIA TRIGGERU PRE KONTROLU ZOSTATKU:');
    DBMS_OUTPUT.PUT_LINE('Účet: ' || v_ucet_id || ', Aktuálny zostatok: ' || v_zostatok);
    DBMS_OUTPUT.PUT_LINE('Pokus o výber čiastky vyšší než je zostatok (' || (v_zostatok + 1000) || ')...');
    
    INSERT INTO Operacia (
        cisloUctu, zadavatel, vykonavatel, ciastka, mena, stav, typ
    ) VALUES (
        v_ucet_id, v_rodne_cislo, v_zamestnanec_id, v_zostatok + 1000, 'EUR', 'ČAKAJÚCA', 'VÝBER'
    ) RETURNING idOperacie INTO v_operace_id;
    
    INSERT INTO Vyber (
        idOperacie, cisloUctu, miesto
    ) VALUES (
        v_operace_id, v_ucet_id, 'Test triggeru'
    );
    
    DECLARE
        v_stav Operacia.stav%TYPE;
    BEGIN
        SELECT stav INTO v_stav
        FROM Operacia
        WHERE idOperacie = v_operace_id;
        
        DBMS_OUTPUT.PUT_LINE('Stav operácie po spustení triggeru: ' || v_stav);
        IF v_stav = 'ZAMIETNUTÁ' THEN
            DBMS_OUTPUT.PUT_LINE('Trigger zamietol operáciu pre nedostatočný zostatok.');
        ELSE
            DBMS_OUTPUT.PUT_LINE('CHYBA: Trigger nezamietol operáciu s nedostatočným zostatkom!');
        END IF;
    END;
END;
/

DECLARE
    v_ucet_id Ucet.cisloUctu%TYPE;
    v_zamestnanec_id Zamestnanec.idZamestnanca%TYPE;
    v_zostatok_pred Ucet.zostatok%TYPE;
    v_zostatok_po Ucet.zostatok%TYPE;
    v_rodne_cislo Klient.rodneCisloKlienta%TYPE;
    v_operace_id Operacia.idOperacie%TYPE;
    v_vklad_castka NUMBER := 500;
BEGIN
    SELECT cisloUctu, zostatok, rodneCisloKlienta INTO v_ucet_id, v_zostatok_pred, v_rodne_cislo
    FROM Ucet
    WHERE ROWNUM = 1;
    
    SELECT idZamestnanca INTO v_zamestnanec_id
    FROM Zamestnanec
    WHERE ROWNUM = 1;
    
    DBMS_OUTPUT.PUT_LINE('');
    DBMS_OUTPUT.PUT_LINE('DEMONŠTRÁCIA TRIGGERU AKTUALIZÁCIE ZOSTATKU:');
    DBMS_OUTPUT.PUT_LINE('Účet: ' || v_ucet_id || ', Zostatok pred vkladom: ' || v_zostatok_pred);
    DBMS_OUTPUT.PUT_LINE('Vkladáme ' || v_vklad_castka || ' EUR...');
    
    INSERT INTO Operacia (
        cisloUctu, zadavatel, vykonavatel, ciastka, mena, stav, typ
    ) VALUES (
        v_ucet_id, v_rodne_cislo, v_zamestnanec_id, v_vklad_castka, 'EUR', 'VYKONANÁ', 'VKLAD'
    ) RETURNING idOperacie INTO v_operace_id;
    
    INSERT INTO Vklad (
        idOperacie, cisloUctu, miesto
    ) VALUES (
        v_operace_id, v_ucet_id, 'Test triggeru'
    );
    
    SELECT zostatok INTO v_zostatok_po
    FROM Ucet
    WHERE cisloUctu = v_ucet_id;
    
    DBMS_OUTPUT.PUT_LINE('Zostatok po vklade: ' || v_zostatok_po);
    IF v_zostatok_po = v_zostatok_pred + v_vklad_castka THEN
        DBMS_OUTPUT.PUT_LINE('Trigger správne aktualizoval zostatok na účte.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('CHYBA: Zostatek nebol správne aktualizovaný!');
    END IF;
END;
/

SET SERVEROUTPUT ON;

BEGIN
    DBMS_OUTPUT.PUT_LINE('');
    DBMS_OUTPUT.PUT_LINE('DEMONŠTRÁCIA PROCEDÚRY PRE GENEROVANIE VÝPISU:');
    p_generuj_vypis('7602151234', TO_DATE('2023-01-01', 'YYYY-MM-DD'), TO_DATE('2023-12-31', 'YYYY-MM-DD'));
END;
/

DECLARE
    v_z_uctu Ucet.cisloUctu%TYPE;
    v_na_ucet Ucet.cisloUctu%TYPE;
    v_suma NUMBER := 100;
    v_rodne_cislo Klient.rodneCisloKlienta%TYPE;
    v_zamestnanec_id Zamestnanec.idZamestnanca%TYPE;
    v_zostatok_pred_odeslani Ucet.zostatok%TYPE;
    v_zostatok_pred_prijeti Ucet.zostatok%TYPE;
    v_zostatok_po_odeslani Ucet.zostatok%TYPE;
    v_zostatok_po_prijeti Ucet.zostatok%TYPE;
    v_id_operace Operacia.idOperacie%TYPE;
BEGIN
    SELECT cisloUctu INTO v_z_uctu
    FROM Ucet
    WHERE ROWNUM = 1;
    
    SELECT cisloUctu INTO v_na_ucet
    FROM Ucet
    WHERE cisloUctu != v_z_uctu AND ROWNUM = 1;
    
    SELECT rodneCisloKlienta INTO v_rodne_cislo
    FROM Ucet
    WHERE cisloUctu = v_z_uctu;
    
    SELECT idZamestnanca INTO v_zamestnanec_id
    FROM Zamestnanec
    WHERE ROWNUM = 1;
    
    SELECT zostatok INTO v_zostatok_pred_odeslani
    FROM Ucet
    WHERE cisloUctu = v_z_uctu;
    
    SELECT zostatok INTO v_zostatok_pred_prijeti
    FROM Ucet
    WHERE cisloUctu = v_na_ucet;
    
    DBMS_OUTPUT.PUT_LINE('');
    DBMS_OUTPUT.PUT_LINE('DEMONŠTRÁCIA PROCEDÚRY PRE PREVOD:');
    DBMS_OUTPUT.PUT_LINE('Z účtu: ' || v_z_uctu || ', zostatok pred: ' || v_zostatok_pred_odeslani);
    DBMS_OUTPUT.PUT_LINE('Na účet: ' || v_na_ucet || ', zostatok pred: ' || v_zostatok_pred_prijeti);
    DBMS_OUTPUT.PUT_LINE('Čiastka: ' || v_suma || ' EUR...');
    
    p_prevod_mezi_ucty(
        p_z_uctu => v_z_uctu,
        p_na_ucet => v_na_ucet,
        p_suma => v_suma,
        p_zadavatel => v_rodne_cislo,
        p_vykonavatel => v_zamestnanec_id,
        p_id_operace => v_id_operace
    );
    
    SELECT zostatok INTO v_zostatok_po_odeslani
    FROM Ucet
    WHERE cisloUctu = v_z_uctu;
    
    SELECT zostatok INTO v_zostatok_po_prijeti
    FROM Ucet
    WHERE cisloUctu = v_na_ucet;
    
    DBMS_OUTPUT.PUT_LINE('Prevod dokončený, ID operácie: ' || v_id_operace);
    DBMS_OUTPUT.PUT_LINE('Z účtu: ' || v_z_uctu || ', zostatok po: ' || v_zostatok_po_odeslani);
    DBMS_OUTPUT.PUT_LINE('Na účet: ' || v_na_ucet || ', zostatok po: ' || v_zostatok_po_prijeti);
    
    IF v_zostatok_po_odeslani = v_zostatok_pred_odeslani - v_suma AND 
       v_zostatok_po_prijeti = v_zostatok_pred_prijeti + v_suma THEN
        DBMS_OUTPUT.PUT_LINE('Procedúra správne spravila prevod a aktualizovala zostatky.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('CHYBA: Zostatky neboli správne aktualizované!');
    END IF;
END;
/

DECLARE
    v_mesto VARCHAR2(255);
    v_typ VARCHAR2(20);
    v_pocet NUMBER;
    v_suma NUMBER;
    v_prumer NUMBER;
    v_velmi_aktivni NUMBER;
    v_vysoky_objem NUMBER;
    
    CURSOR c_vysledky IS
    WITH 
        operace_souhrn AS (
            SELECT 
                K.mesto,
                O.typ,
                COUNT(*) AS pocet_operaci,
                SUM(O.ciastka) AS celkova_suma,
                AVG(O.ciastka) AS prumerna_castka
            FROM 
                Klient K
                JOIN Ucet U ON K.rodneCisloKlienta = U.rodneCisloKlienta
                JOIN Operacia O ON U.cisloUctu = O.cisloUctu
            WHERE 
                O.stav = 'VYKONANÁ'
                AND O.datumCas BETWEEN TO_DATE('2023-01-01', 'YYYY-MM-DD') AND TO_DATE('2023-12-31', 'YYYY-MM-DD')
            GROUP BY 
                K.mesto, O.typ
        ),
        klient_aktivity AS (
            SELECT 
                K.rodneCisloKlienta,
                K.meno || ' ' || K.priezvisko AS jmeno,
                K.mesto,
                COUNT(O.idOperacie) AS pocet_operaci,
                SUM(O.ciastka) AS celkova_suma,
                CASE 
                    WHEN COUNT(O.idOperacie) > 5 THEN 'Veľmi aktívny'
                    WHEN COUNT(O.idOperacie) > 2 THEN 'Aktívny'
                    WHEN COUNT(O.idOperacie) > 0 THEN 'Málo aktívny'
                    ELSE 'Neaktívny'
                END AS aktivita_kategorie,
                CASE 
                    WHEN SUM(O.ciastka) > 10000 THEN 'Vysoký objem'
                    WHEN SUM(O.ciastka) > 5000 THEN 'Stredný objem'
                    WHEN SUM(O.ciastka) > 0 THEN 'Nízky objem'
                    ELSE 'Žiadny objem'
                END AS objem_kategorie
            FROM 
                Klient K
                LEFT JOIN Ucet U ON K.rodneCisloKlienta = U.rodneCisloKlienta
                LEFT JOIN Operacia O ON U.cisloUctu = O.cisloUctu 
                    AND O.stav = 'VYKONANÁ'
                    AND O.datumCas BETWEEN TO_DATE('2023-01-01', 'YYYY-MM-DD') AND TO_DATE('2023-12-31', 'YYYY-MM-DD')
            GROUP BY 
                K.rodneCisloKlienta, K.meno, K.priezvisko, K.mesto
        )
    SELECT 
        os.mesto,
        os.typ,
        os.pocet_operaci,
        os.celkova_suma,
        os.prumerna_castka,
        (SELECT COUNT(*) FROM klient_aktivity ka WHERE ka.mesto = os.mesto AND ka.aktivita_kategorie = 'Veľmi aktívny') AS pocet_velmi_aktivnich,
        (SELECT COUNT(*) FROM klient_aktivity ka WHERE ka.mesto = os.mesto AND ka.objem_kategorie = 'Vysoký objem') AS pocet_vysoky_objem
    FROM 
        operace_souhrn os
    ORDER BY 
        os.mesto, os.pocet_operaci DESC;
BEGIN
    DBMS_OUTPUT.PUT_LINE('');
    DBMS_OUTPUT.PUT_LINE('VÝSLEDKY KOMPLEXNÍHO DOTAZU S WITH A CASE:');
    DBMS_OUTPUT.PUT_LINE('---------------------------------------------------');
    DBMS_OUTPUT.PUT_LINE('MESTO     | TYP     | POČET | SUMA    | PRIEMER   | VEĽMI AKTÍVNY | VYSOKÝ OBJEM');
    DBMS_OUTPUT.PUT_LINE('---------------------------------------------------');
    
    OPEN c_vysledky;
    LOOP
        FETCH c_vysledky INTO v_mesto, v_typ, v_pocet, v_suma, v_prumer, v_velmi_aktivni, v_vysoky_objem;
        EXIT WHEN c_vysledky%NOTFOUND;
        
        DBMS_OUTPUT.PUT_LINE(
            RPAD(v_mesto, 10) || ' | ' ||
            RPAD(v_typ, 8) || ' | ' ||
            LPAD(TO_CHAR(v_pocet), 5) || ' | ' ||
            LPAD(TO_CHAR(ROUND(v_suma, 2)), 8) || ' | ' ||
            LPAD(TO_CHAR(ROUND(v_prumer, 2)), 8) || ' | ' ||
            LPAD(TO_CHAR(v_velmi_aktivni), 13) || ' | ' ||
            LPAD(TO_CHAR(v_vysoky_objem), 12)
        );
    END LOOP;
    CLOSE c_vysledky;
    
    DBMS_OUTPUT.PUT_LINE('---------------------------------------------------');
    DBMS_OUTPUT.PUT_LINE('Poznámka: Dotaz poskytuje analýzu bankových operácií podľa miest a typológie klientov.');
END;
/