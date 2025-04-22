-- uart_rx.vhd: UART controller - receiving (RX) side
-- Author(s): Hugo Bohácsek (xbohach00)

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;


-- Entity declaration (DO NOT ALTER THIS PART!)
entity UART_RX is
    port(
        CLK      : in std_logic;
        RST      : in std_logic;
        DIN      : in std_logic;
        DOUT     : out std_logic_vector(7 downto 0);
        DOUT_VLD : out std_logic
    );
end entity;



-- Architecture implementation (INSERT YOUR IMPLEMENTATION HERE)
architecture behavioral of UART_RX is

    signal CLK_Y    : std_logic;
    signal VALID    : std_logic;
    signal CLK_CT   : std_logic_vector(4 downto 0);
    signal READ_Y   : std_logic;
    signal BITS_CT  : std_logic_vector(3 downto 0);

begin

    -- Instance of RX FSM
    fsm: entity work.UART_RX_FSM
    port map (
        CLK         => CLK,
        DIN         => DIN,
        RST         => RST,
        CLK_Y       => CLK_Y,
        READ_Y      => READ_Y,
        CLK_CNT     => CLK_CT,
        BITS_CNT    => BITS_CT,
        DOUT_VLD    => VALID
    );

    DOUT_VLD <= VALID;

    process(CLK) begin

        if rising_edge(CLK) then
            if RST = '1' then               -- Allow for asynchronous reset
                BITS_CT <= "0000";
                CLK_CT <= "00000";
                DOUT <= "00000000";
            else
                if CLK_Y = '0' then         -- Reset the counter when the clock is low
                    CLK_CT <= "00000";
                else
                    CLK_CT <= CLK_CT + 1;   -- Increment the clock counter
                end if;
                if READ_Y = '0' then        -- Reset the bit counter when the read signal is low
                    BITS_CT <= "0000";
                end if;
                if READ_Y = '1' and CLK_CT(4) = '1' then -- Simulate the Demultiplexer
                    case BITS_CT is
                        when "0000" => DOUT(0) <= DIN;
                        when "0001" => DOUT(1) <= DIN;
                        when "0010" => DOUT(2) <= DIN;
                        when "0011" => DOUT(3) <= DIN;
                        when "0100" => DOUT(4) <= DIN;
                        when "0101" => DOUT(5) <= DIN;
                        when "0110" => DOUT(6) <= DIN;
                        when "0111" => DOUT(7) <= DIN;
                        when others => null;
                    end case;
                    CLK_CT  <= "00001"; -- Reset the clock counter
                    BITS_CT <= BITS_CT + 1; -- Increment the bit counter
                end if;
            end if;
        end if;
    end process;
end architecture;
