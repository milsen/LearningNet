const path = require('path');
const webpack = require('webpack');

const dirAssets = path.join(__dirname, 'assets');
const isProd = process.env.NODE_ENV === 'production';

module.exports = {
    mode: isProd ? 'production' : 'development',
    watch: true,
    devtool: isProd ? 'source-map' : '#eval-source-map',
    entry: {
        index: path.join(dirAssets, 'js', 'index.js'),
        edit: path.join(dirAssets, 'js', 'edit.js'),
        settings: path.join(dirAssets, 'js', 'settings.js')
    },
    output: {
        path: path.join(dirAssets, './dist'),
        chunkFilename: '[name].chunk.js',
        filename: '[name].js'
    },
    module: {
        rules: [
            {
                test: /\.css$/i,
                use: ['style-loader', 'css-loader'],
            },
        ],
    },
    resolve: {
        alias: {
            CSS: path.resolve(__dirname, 'assets/css/'),
            JS: path.resolve(__dirname, 'assets/js/')
        }
    }
};
