const path = require('path');
const webpack = require('webpack');
const ExtractTextPlugin = require('extract-text-webpack-plugin');

const dirAssets = path.join(__dirname, 'assets');
const isProd = process.env.NODE_ENV === 'production';

module.exports = {
    mode: isProd ? 'production' : 'development',
    watch: true,
    devtool: isProd ? 'source-map' : '#eval-source-map',
    entry: {
        learningnet: path.join(dirAssets, 'js', 'learningnet.js')
    },
    output: {
        path: path.join(dirAssets, './dist'),
        chunkFilename: '[name].chunk.js',
        filename: '[name].js'
        // pathinfo: !isProd,
        // publicPath: !isProd ? 'http://localhost:8081/' : undefined
    },
    // module: {
    //   rules: [
    //     {
    //       test: /\.js$/,
    //       exclude: /(node_modules)/,
    //       use: {
    //         loader: 'babel-loader',
    //         options: {
    //           cacheDirectory: true
    //         }
    //       }
    //     },
    //     {
    //       test: /\.less$/,
    //       use: ExtractTextPlugin.extract({
    //         fallback: 'style-loader',
    //         use: [
    //           {
    //             loader: 'css-loader',
    //             options: {
    //               url: false,
    //               sourceMap: true,
    //               importLoaders: 2
    //             }
    //           },
    //           {
    //             loader: 'postcss-loader',
    //             options: {
    //               sourceMap: true
    //             }
    //           },
    //           {
    //             loader: 'less-loader',
    //             options: {
    //               sourceMap: true
    //             }
    //           }
    //         ]
    //       })
    //     }
    //   ]
    // },
    resolve: {
        extensions: [ '.js' ],
        modules: [
            'node_modules',
            dirAssets
        ]
    },
    plugins: [
        //   new webpack.LoaderOptionsPlugin({
        //     minimize: true,
        //     debug: !isProd,
        //     options: {
        //     }
        //   }),
        //   // new webpack.optimize.minimize({
        //   //   comments: !isProd,
        //   //   sourceMap: true
        //   // }),
        //   new ExtractTextPlugin('learningnet.css')
    ],
    // externals: {
    //   jquery: 'jQuery'
    // }
};
