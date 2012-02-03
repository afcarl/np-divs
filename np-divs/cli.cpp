#include "np-divs/np_divs.hpp"
#include "np-divs/matrix_io.hpp"
#include "np-divs/div-funcs/div_func.hpp"
#include "np-divs/div_func_from_str.hpp"

#include <iostream>
#include <fstream>
#include <string>

#include <boost/program_options.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/utility.hpp>

#include <flann/util/matrix.h>

using namespace NPDivs;
using namespace std;
using namespace boost;
namespace po = boost::program_options;

typedef struct s_popts : noncopyable {
    string x_bags_file;
    string y_bags_file;
    string results_file;

    ptr_vector<DivFunc> div_funcs;

    size_t k;
    size_t num_threads;

    flann::IndexParams index_params;
    flann::SearchParams search_params;

    void parse_div_funcs(const vector<string> &names) {
        for (size_t i = 0; i < names.size(); i++) {
            div_funcs.push_back(div_func_from_str(names[i]));
        }
    }

    void parse_index(const string name) {
        // TODO: more index types, support arguments
        if (name == "linear" || name == "brute") {
            index_params = flann::LinearIndexParams();

        } else if (name == "kdtree" || name == "kd") {
            index_params = flann::KDTreeSingleIndexParams();

        } else {
            throw domain_error((format("unknown index type %s") % name).str());
        }
    }
} ProgOpts;


bool parse_args(int argc, const char ** argv, ProgOpts& opts);

int main(int argc, const char ** argv) {
    typedef flann::Matrix<double> Matrix;

    ProgOpts opts;
    opts.search_params = flann::SearchParams(64);
    if (!parse_args(argc, argv, opts))
        return 1;

    size_t num_df = opts.div_funcs.size();

    // load input bags
    // TODO - gracefully handle nonexisting files
    size_t num_x;
    Matrix* x_bags;
    if (opts.x_bags_file == "-") {
        x_bags = matrices_from_csv(cin, num_x);
    } else {
        ifstream ifs(opts.x_bags_file.c_str(), ifstream::in);
        x_bags = matrices_from_csv(ifs, num_x);
    }

    size_t num_y;
    Matrix* y_bags;
    if (opts.y_bags_file.empty()) {
        y_bags = NULL;
        num_y = num_x;
    } else if (opts.y_bags_file == "-") {
        y_bags = matrices_from_csv(cin, num_y);
    } else {
        ifstream ifs(opts.y_bags_file.c_str(), ifstream::in);
        y_bags = matrices_from_csv(ifs, num_y);
    }


    Matrix* results = alloc_matrix_array<double>(num_df, num_x, num_y);

    try {
        np_divs(x_bags, num_x, y_bags, num_y, opts.div_funcs, results, opts.k,
                opts.index_params, opts.search_params, opts.num_threads);
    } catch (std::exception e) {
        cerr << "Error: " << e.what() << endl;
        exit(1);
    }

    if (opts.results_file == "-") {
        matrix_array_to_csv(cout, results, num_df);
    } else {
        ofstream ofs(opts.results_file.c_str());
        matrix_array_to_csv(ofs, results, num_df);
    }

    free_matrix_array(results, num_df);

    return 0;
}


// TODO nicer handling of matrix inputs
// TODO optionally support HDF5 inputs
// TODO positional arguments for x_bags, y_bags
// TODO support setting {index,search}_params
bool parse_args(int argc, const char ** argv, ProgOpts& opts) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Produce this help message.")
        ("x-bags,x",
            po::value<string>(&opts.x_bags_file)->default_value("-"),
            "CSV-style file containing matrices separated by blank lines; "
            "- means stdin.")
        ("y-bags,y",
            po::value<string>(&opts.y_bags_file),
            "CSV-style file containing matrices separated by blank lines. "
            "If passed, divergences are calculated between the x bags (rows "
            "in the output matrix) and the y bags (columns); if not passed, "
            "divergences are calculated from the x bags to themselves. Pass "
            "- to use stdin; if both x and y are from stdin, x is read first "
            "and the two groups must be separated by exactly two blank lines. ")
        ("results,r",
            po::value<string>(&opts.results_file)->default_value("-"),
            "Where to write CSV-style matrix of results, where m[i,j] = "
            "div(x_i, y_j). Use - for stdout.")
        ("div-func,f",
            po::value< vector<string> >()->composing()->required()
               ->notifier(bind(&ProgOpts::parse_div_funcs, ref(opts), _1)),
            "Divergence functions to use; can be specified more than once. At "
            "least one is required. Format is name:arg1:arg2:..., where argN "
            "refers to the Nth argument to the corresponding DivFunc's "
            "constructor. Some support a first argument specifying a "
            "parameter: renyi:.99 means the Renyi-.99 divergence. All support "
            "a last parameter, which determines the way that large values are "
            "normalized: l2:.95 or renyi:.99:.95 means certain calculated "
            "intermediate values above the 95th percentile are cut down; 1 "
            "means not to do this; default is .99. All extra arguments are "
            "optional.")
        ("num-threads",
            po::value<size_t>(&opts.num_threads)->default_value(0),
            "Number of threads to use for calculations. 0 means one per core.")
        ("neighbors,k",
            po::value<size_t>(&opts.k)->default_value(3),
            "The k for k-nearest-neighbor calculations.")
        ("index,i",
            po::value<string>()->default_value("kdtree")
                ->notifier(bind(&ProgOpts::parse_index, ref(opts), _1)),
            "The nearest-neighbor index to use. Options: linear, kdtree.")
    ;

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            exit(0);
        }

        po::notify(vm);

    } catch (std::exception &e) {
        cerr << "Error: " << e.what() << endl;
        return false;
    }

    return true;
}
