-- cpu.vhd: Simple 8-bit CPU (BrainFuck interpreter)
-- Copyright (C) 2024 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Hugo Bohácsek <xbohach00 AT stud.fit.vutbr.cz>
--
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
 port (
   CLK   : in std_logic;  -- hodinovy signal
   RESET : in std_logic;  -- asynchronni reset procesoru
   EN    : in std_logic;  -- povoleni cinnosti procesoru
 
   -- synchronni pamet RAM
   DATA_ADDR  : out std_logic_vector(12 downto 0); -- adresa do pameti
   DATA_WDATA : out std_logic_vector(7 downto 0); -- mem[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
   DATA_RDATA : in std_logic_vector(7 downto 0);  -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
   DATA_RDWR  : out std_logic;                    -- cteni (1) / zapis (0)
   DATA_EN    : out std_logic;                    -- povoleni cinnosti
   
   -- vstupni port
   IN_DATA   : in std_logic_vector(7 downto 0);   -- IN_DATA <- stav klavesnice pokud IN_VLD='1' a IN_REQ='1'
   IN_VLD    : in std_logic;                      -- data platna
   IN_REQ    : out std_logic;                     -- pozadavek na vstup data
   
   -- vystupni port
   OUT_DATA : out  std_logic_vector(7 downto 0);  -- zapisovana data
   OUT_BUSY : in std_logic;                       -- LCD je zaneprazdnen (1), nelze zapisovat
   OUT_INV  : out std_logic;                      -- pozadavek na aktivaci inverzniho zobrazeni (1)
   OUT_WE   : out std_logic;                      -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'

   -- stavove signaly
   READY    : out std_logic;                      -- hodnota 1 znamena, ze byl procesor inicializovan a zacina vykonavat program
   DONE     : out std_logic                       -- hodnota 1 znamena, ze procesor ukoncil vykonavani programu (narazil na instrukci halt)
 );
end cpu;


-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of cpu is
    -- Definícia cpu signálov podľa schémy v zadaní
    signal pc_reg: std_logic_vector(12 downto 0);
    signal pc_inc: std_logic;
    signal pc_dec: std_logic;
  
    signal ptr_reg: std_logic_vector(12 downto 0);
    signal ptr_inc: std_logic;
    signal ptr_dec: std_logic;
  
    signal tmp_reg: std_logic_vector(7 downto 0);
    signal tmp_ld: std_logic;
  
    signal cnt_reg: std_logic_vector(7 downto 0);
    signal cnt_inc: std_logic;
    signal cnt_dec: std_logic;

    signal mx1_sel: std_logic;

    signal mx2_sel: std_logic_vector(1 downto 0);

    type type_fsm_state is (
      FSM_START,
        -- Stavy inicializácie procesoru
      FSM_INIT,
      FSM_INIT_INC,
      FSM_INIT_READY,
        -- Stavy načítavania do programu
      FSM_FETCH,
      FSM_DECODE,
        -- Stavy popisujúce inštrukcie > a <
      FSM_PTR_INC,
      FSM_PTR_DEC,
        -- Stavy popisujúce inštrukcie + a -
      FSM_MEM_INC,
      FSM_MEM_INC_W,
      FSM_MEM_DEC,
      FSM_MEM_DEC_W,
        -- Stavy popisujúce inštrukcie [ a ]
      FSM_LOOP_START,
      FSM_LOOP_COMPARE,
      FSM_LOOP_COMPARE_INC,
      FSM_LOOP_READ_INSTRUCTION,
      FSM_LOOP_CHECK_INSTRUCTION,
      FSM_LOOP_CNT_INC,
      FSM_LOOP_CNT_DEC,
      FSM_LOOP_CHECK_CNT,
      FSM_LOOP_END,
      FSM_LOOP_END_COMPARE,
      FSM_LOOP_END_COMPARE_INC,
      FSM_LOOP_END_PC_INC,
      FSM_LOOP_END_READ_INSTRUCTION,
      FSM_LOOP_END_CHECK_INSTRUCTION,
      FSM_LOOP_END_CNT_INC,
      FSM_LOOP_END_CNT_DEC,
      FSM_LOOP_END_CHECK_CNT,
      FSM_LOOP_END_PC_DEC,
      FSM_LOOP_END_NFETCH,
        -- Stavy popisujúce inštrukcie , a .
      FSM_TMP_START,
      FSM_TMP_LD,
      FSM_TMP_MEM,
      FSM_PRINT_START,
      FSM_PRINT_WAIT,
      FSM_PRINT,
      FSM_MEM_WRITE,
      FSM_MEM_WRITE_VALID,
      FSM_HALT,
      FSM_OTHERS);
      -- Momentálny a následujúci stav automatu
    signal fsm_state: type_fsm_state;
    signal next_state: type_fsm_state;
begin

 -- pri tvorbe kodu reflektujte rady ze cviceni INP, zejmena mejte na pameti, ze 
 --   - nelze z vice procesu ovladat stejny signal,
 --   - je vhodne mit jeden proces pro popis jedne hardwarove komponenty, protoze pak
 --      - u synchronnich komponent obsahuje sensitivity list pouze CLK a RESET a 
 --      - u kombinacnich komponent obsahuje sensitivity list vsechny ctene signaly. 

  -- Posúvanie sa po programe
  pc_proc: process (RESET, CLK, pc_inc, pc_dec)
  begin

    -- Resetovanie registra -> vraciame sa na začiatok programu
    if RESET = '1' then
      pc_reg <= (OTHERS => '0');
    

    -- Ak je CLK
    elsif CLK'event and CLK = '1' then
        -- Posúvame sa v pravo - ovládané loopmi
        if pc_inc = '1' then
            pc_reg <= pc_reg + 1;
        end if;

        -- Posúvame sa vľavo
        if pc_dec = '1' then
            pc_reg <= pc_reg - 1;
        end if;
    end if;

  end process;

  -- Posúvanie sa po pamäti programu
  ptr_proc: process (RESET, CLK, ptr_reg, ptr_inc, ptr_dec)
  begin

    -- Klasický reset
    if RESET = '1' then
      ptr_reg <= (OTHERS => '0');
    

    elsif CLK'event and CLK = '1' then
        -- Posúvame sa doprava, ovládané znakom >
        if ptr_inc = '1' then
            if ptr_reg = "1111111111111" then
                ptr_reg <= (OTHERS => '0');
            else
                ptr_reg <= ptr_reg + 1;
            end if;
        end if;

        -- Posúvame sa doľava, ovládané znakom <
        if ptr_dec = '1' then
            if ptr_reg = "0000000000000" then
                ptr_reg <= (OTHERS => '1');
            else
                ptr_reg <= ptr_reg - 1;
            end if;
        end if;
    end if;

  end process;

  -- Uchovanie hodnoty do dočasnej premennej
  tmp_proc: process (RESET, CLK, tmp_ld, DATA_RDATA)
  begin

    if RESET = '1' then
      tmp_reg <= (OTHERS => '0');
    
    
    elsif CLK'event and CLK = '1' then
          -- Len pokiaľ sme mali znak $
        if tmp_ld = '1' then
            tmp_reg <= DATA_RDATA;
        end if;
    end if;
  
  end process;

  -- Register CNT - počíta zátvorky cyklov
  cnt_proc: process (RESET, CLK, cnt_inc, cnt_dec)
  begin

    if RESET = '1' then
      cnt_reg <= (OTHERS => '0');
    
    
    elsif CLK'event and CLK = '1' then
          -- Počas vykonávania programu v cykle ideme >
        if cnt_inc = '1' then
            cnt_reg <= cnt_reg + 1;
        end if;
    
          -- Keď sa vraciame na začiatok cyklu, bez vykonávania programu ideme <
        if cnt_dec = '1' then
            cnt_reg <= cnt_reg - 1;
        end if;
    end if;
    
  end process;

  -- Multiplexor 1 - určuje druh čítaných dát (adresa programu / pamäte)
  DATA_ADDR <= ptr_reg when mx1_sel = '0' else pc_reg when mx1_sel = '1';

  -- Multiplexor 2 - určuje hodnotu zápisu do pamäte
  DATA_WDATA <= IN_DATA when mx2_sel = "00" else
                tmp_reg when mx2_sel = "01" else
                DATA_RDATA-1 when mx2_sel = "10" else
                DATA_RDATA+1 when mx2_sel = "11";

  -- Posun stavov automatu
  fsm_reg: process (RESET, CLK)
  begin
    if CLK'event and CLK = '1' then
      if RESET = '1' then
          fsm_state <= FSM_START;
      end if;

      if RESET = '0' then
          fsm_state <= next_state;
      end if;
    end if;
  end process;

  -- FSM nstate logic
  next_state_logic: process (fsm_state, EN, IN_VLD, OUT_BUSY, DATA_RDATA, cnt_reg)
  begin
    if EN = '1' then
      case fsm_state is
        when FSM_START | FSM_INIT_INC=>
          next_state <= FSM_INIT;

        when FSM_INIT =>
          if DATA_RDATA = X"40" then
            next_state <= FSM_INIT_READY;
          else
            next_state <= FSM_INIT_INC;
          end if;

        when FSM_INIT_READY | FSM_PTR_INC | FSM_PTR_DEC | FSM_MEM_INC_W | FSM_MEM_DEC_W | FSM_TMP_LD | FSM_TMP_MEM | FSM_PRINT | FSM_MEM_WRITE_VALID | FSM_LOOP_END_PC_INC | FSM_LOOP_END_NFETCH | FSM_OTHERS =>
            next_state <= FSM_FETCH;

        when FSM_FETCH =>
          next_state <= FSM_DECODE;

        when FSM_DECODE =>
          case DATA_RDATA is
            when X"3E" => -- >
              next_state <= FSM_PTR_INC;
            when X"3C" => -- <
              next_state <= FSM_PTR_DEC;
            when X"2B" => -- +
              next_state <= FSM_MEM_INC;
            when X"2D" => -- -
              next_state <= FSM_MEM_DEC;
            when X"5B" => -- [
              next_state <= FSM_LOOP_START;
            when X"5D" => -- ]
              next_state <= FSM_LOOP_END;
            when X"24" => -- $
              next_state <= FSM_TMP_START;
            when X"21" => -- !
              next_state <= FSM_TMP_MEM;
            when X"2E" => -- .
              next_state <= FSM_PRINT_START;
            when X"2C" => -- ,
              next_state <= FSM_MEM_WRITE;
            when X"40" => -- @
              next_state <= FSM_HALT;
            when others =>
              next_state <= FSM_OTHERS;
          end case;

        when FSM_MEM_INC =>
          next_state <= FSM_MEM_INC_W;

        when FSM_MEM_DEC =>
          next_state <= FSM_MEM_DEC_W;

        when FSM_TMP_START =>
          next_state <= FSM_TMP_LD;


        when FSM_PRINT_START =>
          next_state <= FSM_PRINT_WAIT;

        when FSM_PRINT_WAIT =>
            if OUT_BUSY = '1' then
              next_state <= FSM_PRINT_WAIT;
            else
              next_state <= FSM_PRINT;
            end if;

        when FSM_MEM_WRITE =>
            if IN_VLD = '0' then
              next_state <= FSM_MEM_WRITE;
            else
              next_state <= FSM_MEM_WRITE_VALID;
            end if;

        when FSM_HALT =>
          next_state <= FSM_HALT;

        when FSM_LOOP_START =>
          next_state <= FSM_LOOP_COMPARE;

        when FSM_LOOP_COMPARE =>
            if DATA_RDATA = X"00" then
              next_state <= FSM_LOOP_COMPARE_INC;
            else
              next_state <= FSM_FETCH;
            end if;

        when FSM_LOOP_COMPARE_INC =>
          next_state <= FSM_LOOP_READ_INSTRUCTION;

        when FSM_LOOP_READ_INSTRUCTION =>
          next_state <= FSM_LOOP_CHECK_INSTRUCTION;

        when FSM_LOOP_CHECK_INSTRUCTION =>
            if DATA_RDATA <= X"5B" then
              next_state <= FSM_LOOP_CNT_INC;
            elsif DATA_RDATA <= X"5D" then
              next_state <= FSM_LOOP_CNT_DEC;
            else
              next_state <= FSM_LOOP_CHECK_CNT;
            end if;

        when FSM_LOOP_CNT_INC | FSM_LOOP_CNT_DEC=>
          next_state <= FSM_LOOP_CHECK_CNT;

        when FSM_LOOP_CHECK_CNT =>
          if cnt_reg = "00000000" then
            next_state <= FSM_FETCH;
          else
            next_state <= FSM_LOOP_READ_INSTRUCTION;
          end if;

        when FSM_LOOP_END =>
          next_state <= FSM_LOOP_END_COMPARE;

        when FSM_LOOP_END_COMPARE =>
          if DATA_RDATA = X"00" then
            next_state <= FSM_LOOP_END_PC_INC;
          else
            next_state <= FSM_LOOP_END_COMPARE_INC;
          end if;

        when FSM_LOOP_END_COMPARE_INC | FSM_LOOP_END_PC_DEC=>
          next_state <= FSM_LOOP_END_READ_INSTRUCTION;

        when FSM_LOOP_END_READ_INSTRUCTION =>
          next_state <= FSM_LOOP_END_CHECK_INSTRUCTION;

        when FSM_LOOP_END_CHECK_INSTRUCTION =>
          if DATA_RDATA = X"5B" then
            next_state <= FSM_LOOP_END_CNT_DEC;
          elsif DATA_RDATA = X"5D" then
            next_state <= FSM_LOOP_END_CNT_INC;
          else
            next_state <= FSM_LOOP_END_CHECK_CNT;
          end if;

        when FSM_LOOP_END_CNT_INC | FSM_LOOP_END_CNT_DEC =>
          next_state <= FSM_LOOP_END_CHECK_CNT;

        when FSM_LOOP_END_CHECK_CNT =>
          if cnt_reg = "00000000" then
            next_state <= FSM_LOOP_END_NFETCH;
          else
            next_state <= FSM_LOOP_END_PC_DEC;
          end if;

        when others => null;
      end case;
    end if;
  end process;

  -- Nastavovanie signálov podľa stavu automatu
  fsm_output_logic: process (fsm_state)
  begin

    -- Najskôr je potrebné všetko nastaviť na pôvodné hodnoty
    READY     <= '1';
    DONE      <= '0';

    DATA_RDWR <= '1';
    DATA_EN   <= '0';

    IN_REQ    <= '0';

    OUT_WE    <= '0';
    OUT_INV   <= '0';

    tmp_ld    <= '0';

    ptr_inc   <= '0';
    ptr_dec   <= '0';

    pc_inc    <= '0';
    pc_dec    <= '0';

    cnt_inc   <= '0';
    cnt_dec   <= '0';

    mx1_sel   <= '0';
    mx2_sel   <= "00";

    OUT_DATA  <= DATA_RDATA;

    -- Následne môžeme určiť, čo sa má robiť v jednotlivých stavoch
    case fsm_state is
      when FSM_START | FSM_INIT =>
        READY <= '0';
        DATA_EN <= '1';
      
      when FSM_INIT_INC =>
        ptr_inc <= '1';
        READY <= '0';

      when FSM_FETCH | FSM_DECODE | FSM_LOOP_READ_INSTRUCTION | FSM_LOOP_END_READ_INSTRUCTION =>
        DATA_EN <= '1';
        mx1_sel <= '1';

      when FSM_PTR_INC =>
        ptr_inc <= '1';
        pc_inc <= '1';

      when FSM_PTR_DEC =>
        ptr_dec <= '1';
        pc_inc <= '1';

      when FSM_MEM_INC | FSM_MEM_DEC | FSM_TMP_START | FSM_PRINT_START | FSM_PRINT_WAIT | FSM_LOOP_END =>
        DATA_EN <= '1';

      when FSM_MEM_INC_W =>
        DATA_EN <= '1';
        DATA_RDWR <= '0';
        mx2_sel <= "11";
        pc_inc <= '1';

      when FSM_MEM_DEC_W =>
        DATA_EN <= '1';
        DATA_RDWR <= '0';
        mx2_sel <= "10";
        pc_inc <= '1';

      when FSM_TMP_LD =>
        DATA_EN <= '1';
        tmp_ld <= '1';
        pc_inc <= '1';

      when FSM_TMP_MEM =>
        DATA_EN <= '1';
        DATA_RDWR <= '0';
        mx2_sel <= "01";
        pc_inc <= '1';


      when FSM_PRINT =>
        DATA_EN <= '1';
        OUT_WE <= '1';
        pc_inc <= '1';

      when FSM_MEM_WRITE =>
        IN_REQ <= '1';

      when FSM_MEM_WRITE_VALID =>
        IN_REQ <= '1';
        DATA_EN <= '1';
        DATA_RDWR <= '0';
        mx2_sel <= "00";
        pc_inc <= '1';

      when FSM_LOOP_START =>
        DATA_EN <= '1';
        pc_inc <= '1';

      when FSM_LOOP_COMPARE_INC | FSM_LOOP_END_CNT_INC | FSM_LOOP_CNT_INC =>
        cnt_inc <= '1';

      when FSM_LOOP_CHECK_INSTRUCTION | FSM_LOOP_END_PC_INC | FSM_LOOP_END_NFETCH | FSM_OTHERS =>
        pc_inc <= '1';

      when FSM_LOOP_CNT_DEC | FSM_LOOP_END_CNT_DEC =>
        cnt_dec <= '1';

      when FSM_LOOP_END_COMPARE_INC =>
        cnt_inc <= '1';
        pc_dec <= '1';

      when FSM_LOOP_END_PC_DEC =>
        pc_dec <= '1';

      when FSM_HALT =>
        DONE <= '1';

      when others => null;
    end case;
  end process;

end behavioral;

