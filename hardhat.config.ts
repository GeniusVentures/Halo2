import * as dotenv from "dotenv";

import { HardhatUserConfig, task } from "hardhat/config";
import "@nomiclabs/hardhat-etherscan";
import "@nomiclabs/hardhat-waffle";
import "hardhat-diamond-abi";
import "hardhat-abi-exporter";
import "@typechain/hardhat";
import "hardhat-gas-reporter";
import "solidity-coverage";
import "@nomiclabs/hardhat-ethers";

dotenv.config();

// This is a sample Hardhat task. To learn how to create your own go to
// https://hardhat.org/guides/create-task.html
task("accounts", "Prints the list of accounts", async (taskArgs, hre) => {
    const accounts = await hre.ethers.getSigners();

    for (const account of accounts) {
        console.log(account.address);
    }
});

const elementSeenSet = new Set<string>();
// filter out duplicate function signatures
function genSignature (name: string, inputs: Array<any>, type: string): string {
    return `${type} ${name}(${inputs.reduce((previous, key) => {
        const comma = previous.length ? ',' : '';
        return previous + comma + key.internalType;
    }, '')})`;
}

function filterDuplicateFunctions (abiElement: any, index: number, fullAbiL: any[], fullyQualifiedName: string) {
    if (["function", "event"].includes(abiElement.type)) {
        const funcSignature = genSignature(abiElement.name, abiElement.inputs, abiElement.type);
        if (elementSeenSet.has(funcSignature)) {
            return false;
        }
        elementSeenSet.add(funcSignature);
    } else if (abiElement.type === 'fallback') {
        if (!fullyQualifiedName.match("Halo2Diamond\.sol")) {
            return false;
        }
    } else if (abiElement.type === 'event') {

    }

    return true;
}

// You need to export an object to set up your config
// Go to https://hardhat.org/config/ to learn more

const config: HardhatUserConfig = {
    solidity: {
        version: "0.8.17",
        settings: {
            optimizer: {
                enabled: true,
                runs: 1000
            }
        }
    },
    networks: {
        ropsten: {
            url: process.env.ROPSTEN_URL || "",
            accounts:
        process.env.PRIVATE_KEY !== undefined ? [process.env.PRIVATE_KEY] : []
        },
        mumbai: {
            url: "https://matic-mumbai.chainstacklabs.com",
            accounts: process.env.PRIVATE_KEY !== undefined ? [process.env.PRIVATE_KEY] : []
        },
        polygon: {
            url: `https://polygon-mainnet.infura.io/v3/${process.env.INFURA_API_KEY}`,
            accounts: process.env.PRIVATE_KEY !== undefined ? [process.env.PRIVATE_KEY] : []
        },
        mainnet: {
            url: `https://mainnet.infura.io/v3/${process.env.INFURA_API_KEY}`,
            accounts: process.env.PRIVATE_KEY !== undefined ? [process.env.PRIVATE_KEY] : []
        }

    },
    gasReporter: {
        enabled: process.env.REPORT_GAS !== undefined,
        currency: "USD"
    },
    etherscan: {

        apiKey: {
            goerli: process.env.ETHERSCAN_API_KEY !== undefined ? process.env.ETHERSCAN_API_KEY : '',
            polygonMumbai: process.env.POLYGONSCAN_API_KEY !== undefined ? process.env.POLYGONSCAN_API_KEY : '',
            polygon: process.env.POLYGONSCAN_API_KEY !== undefined ? process.env.POLYGONSCAN_API_KEY : ''
        }
    },
    abiExporter: {
        flat: true,
        spacing: 2,
        pretty: true
    },
    diamondAbi: {
        name: "HALO2Diamond",
        strict: false,
        exclude: ["hardhat-diamond-abi\/.*"],
        filter: filterDuplicateFunctions
    }
};

export default config;
