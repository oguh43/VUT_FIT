/** 
 * @module ForgeConfig
 * @description Configures electron-forge.
 * @author Hugo Bohácsek (xbohach00) 
 * @author Filip Jenis (xjenisf00)
*/
const { FusesPlugin } = require('@electron-forge/plugin-fuses');
const { FuseV1Options, FuseVersion } = require('@electron/fuses');

module.exports = {
	packagerConfig: {
		asar: true,
		icon: 'src/calc_ico512.png',
		extraResources: ['./dokumentace.pdf.txt']
	},
	rebuildConfig: {},
	makers: [
		{
			name: '@electron-forge/maker-deb',
			platforms: ['linux'],
			config: {
				options: {
					name: "kalk",
					icon: "src/calc_ico512.png",
					productName: "Kalk",
					description: "A simple calculator with GUI",
					maintainer: "Filip Jenis <xjenisf00@stud.fit.vutbr.cz>",
					version: "1.0.0"
				}
			},
		},
	],
	plugins: [
		{
			name: '@electron-forge/plugin-auto-unpack-natives',
			config: {},
		},
		// Fuses are used to enable/disable various Electron functionality
		// at package time, before code signing the application
		new FusesPlugin({
			version: FuseVersion.V1,
			[FuseV1Options.RunAsNode]: false,
			[FuseV1Options.EnableCookieEncryption]: true,
			[FuseV1Options.EnableNodeOptionsEnvironmentVariable]: false,
			[FuseV1Options.EnableNodeCliInspectArguments]: false,
			[FuseV1Options.EnableEmbeddedAsarIntegrityValidation]: true,
			[FuseV1Options.OnlyLoadAppFromAsar]: true,
		}),
	],
};
