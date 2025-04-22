-- uart_rx_fsm.vhd: UART controller - finite state machine controlling RX side
-- Author(s): Hugo Bohácsek (xbohach00)

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;


entity UART_RX_FSM is
    port(
        CLK          : in  std_logic;
        DIN          : in  std_logic;                   
        RST          : in  std_logic;
        CLK_Y        : out std_logic;  
        READ_Y       : out std_logic;                   
        CLK_CNT      : in  std_logic_vector(4 downto 0); 
        BITS_CNT     : in  std_logic_vector(3 downto 0); 
        DOUT_VLD     : out std_logic               
    );
end entity;


architecture behavioral of UART_RX_FSM is

    type state is (IDLE, WAIT_START, GET_BITS, WAIT_STOP, VALIDATE); -- States of the state machine
    signal STATE_NOW : state := IDLE; -- Current state of the state machine

    begin

    process(CLK, RST) begin
        if rising_edge(CLK) then
            if RST = '1' then -- Allow for asynchronous reset
                STATE_NOW <= IDLE;
            else 
                case STATE_NOW is           -- State machine
                    when IDLE =>            -- Waiting for transmission to start
                        CLK_Y       <= '0';
                        READ_Y      <= '0';
                        DOUT_VLD    <= '0';
                        if DIN = '0' then
                            STATE_NOW <= WAIT_START; -- Go to waiting for start bit
                        end if;

                    when WAIT_START =>      -- Waiting for start bit to end
                        CLK_Y       <= '1';
                        READ_Y      <= '0';
                        DOUT_VLD    <= '0';
                        if CLK_CNT = "10000" then
                            STATE_NOW <= GET_BITS; -- If start bit is received, start reading bits
                        end if;

                    when GET_BITS =>        -- Reading bits
                        CLK_Y       <= '1';
                        READ_Y      <= '1';
                        DOUT_VLD    <= '0';
                        if BITS_CNT = "1000" then
                            STATE_NOW <= WAIT_STOP; -- If all bits are read, wait for stop bit
                        end if;
                    when WAIT_STOP =>    -- Waiting for stop bit
                        CLK_Y       <= '1';
                        READ_Y      <= '0';
                        DOUT_VLD    <= '0';
                        if CLK_CNT = "10000" then
                            STATE_NOW <= VALIDATE; -- If stop bit is received, validate data
                        end if;
                    when VALIDATE =>    -- Validating received data
                        CLK_Y      <= '0';
                        READ_Y     <= '0';
                        DOUT_VLD   <= '1';
                        STATE_NOW  <= IDLE; -- Return to IDLE state
                end case;
            end if;
        end if;
    end process;
end architecture;
