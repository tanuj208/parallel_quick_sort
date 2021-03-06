/* MPI Program Template */

#include <stdio.h>
#include <string.h>
#include "mpi.h"
#include <bits/stdc++.h>

using namespace std;

typedef long long ll;

#define pb push_back
#define send_data_tag 2001
#define sz(a) (ll)(a.size())
#define mp make_pair
#define ff first
#define ss second
#define pii pair<ll,ll>

ll partition(vector<ll> &arr, ll l, ll r)
{
	ll pivot_idx = rand() % (r-l+1) + l;
	swap(arr[pivot_idx], arr[r]);
	ll pivot_val = arr[r];

	ll i=l-1,j;
	for(j=l;j<r;j++)
	{
		if(arr[j]<pivot_val)
		{
			i++;
			swap(arr[i], arr[j]);
		}
	}
	swap(arr[r], arr[i+1]);
	return i+1;
}

void quick_sort(vector<ll> &arr, ll l, ll r)
{
	if(l<r)
	{
		ll pivot_idx = partition(arr, l, r);
		quick_sort(arr, l, pivot_idx-1);
		quick_sort(arr, pivot_idx+1, r);
	}
}

vector<ll> merge(vector<vector<ll>> &arr)
{
	ll i;
	vector<ll> sorted_arr;
	priority_queue<pair<ll,pii>, vector<pair<ll,pii>>, greater<pair<ll,pii>>> p;
	for(i=0;i<sz(arr);i++)
		p.push(mp(arr[i][0], mp(i,0)));

	while(!p.empty())
	{
		pair<ll,pii> min_el = p.top();
		p.pop();
		sorted_arr.pb(min_el.ff);
		ll arr_idx = min_el.ss.ff;
		ll el_idx = min_el.ss.ss;

		if(el_idx+1 != sz(arr[arr_idx]))
		{
			el_idx++;
			p.push(mp(arr[arr_idx][el_idx], mp(arr_idx, el_idx)));
		}
	}
	return sorted_arr;
}

void take_input(ifstream& infile, ll& n, vector<ll>& arr)
{
	while(infile)
	{
		string data;
		infile>>data;
		if(data=="")
			break;
		arr.pb(stoll(data));
		n++;
	}
	return;
}

void send_chunks(ll n, vector<ll>& arr, ll numprocs)
{
	ll seg_len = n/numprocs;
    for(ll pid = 1;pid < numprocs;pid++)
    {
    	ll start_idx = seg_len*(pid-1);
	    ll numbers_cnt;
    	if(pid==numprocs-1)
    		numbers_cnt = n - start_idx;
    	else
	    	numbers_cnt = seg_len*pid - start_idx;

	    MPI_Send(&numbers_cnt, 1, MPI_LONG_LONG, pid, send_data_tag, MPI_COMM_WORLD);
	    MPI_Send(&arr[start_idx], numbers_cnt, MPI_LONG_LONG, pid, send_data_tag, MPI_COMM_WORLD);
    }
    return;
}

vector<vector<ll>> receive_sorted_arrays(ll numprocs, ll n)
{
	MPI_Status status;
	vector<vector<ll>> rec_arrs;
	ll seg_len = n/numprocs;
    for(ll pid = 1;pid < numprocs;pid++)
    {
    	ll start_idx = seg_len*(pid-1);
	    ll numbers_cnt;
    	if(pid==numprocs-1)
    		numbers_cnt = n - start_idx;
    	else
	    	numbers_cnt = seg_len*pid - start_idx;

	    vector<ll> rec_tmp(numbers_cnt);
	    MPI_Recv(&rec_tmp[0], numbers_cnt, MPI_LONG_LONG, pid, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	    rec_arrs.pb(rec_tmp);
    }
    return rec_arrs;
}

void store_output(ofstream& outfile, vector<ll> &sorted_arr)
{
    for(ll i=0;i<sz(sorted_arr);i++)
    	outfile<<sorted_arr[i]<<" ";
    outfile<<endl;
}

int main( int argc, char **argv ) {
	srand (time(NULL));
	int rank, numprocs;
	int root_process = 0;
	MPI_Status status;
	ll i;
    /* start up MPI */
    MPI_Init( &argc, &argv );

    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &numprocs );
    
    /*synchronize all processes*/
    MPI_Barrier( MPI_COMM_WORLD );
    double tbeg = MPI_Wtime();

    /* write your code here */

    if(rank==root_process)
    {
    	ifstream infile;
    	infile.open(argv[2]);
    	ll n=0;
    	ll i;
    	vector<ll> arr;

    	take_input(infile, n, arr);
    	send_chunks(n, arr, numprocs);
	    MPI_Barrier( MPI_COMM_WORLD );
	    vector<vector<ll>> rec_arrs = receive_sorted_arrays(numprocs, n);
	    vector<ll> sorted_arr = merge(rec_arrs);

	   	ofstream outfile;
	   	outfile.open(argv[3]);
	   	store_output(outfile, sorted_arr);
    }
    else
    {
    	ll seg_len;
    	MPI_Recv(&seg_len, 1, MPI_LONG_LONG, root_process, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    	vector<ll> arr(seg_len+1);
    	MPI_Recv(&arr[0], seg_len, MPI_LONG_LONG, root_process, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    	quick_sort(arr, 0, seg_len-1);
	    MPI_Barrier( MPI_COMM_WORLD );

    	MPI_Send(&arr[0], seg_len, MPI_LONG_LONG, root_process, send_data_tag, MPI_COMM_WORLD);
    }



    MPI_Barrier( MPI_COMM_WORLD );
    double elapsedTime = MPI_Wtime() - tbeg;
    double maxTime;
    MPI_Reduce( &elapsedTime, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
    if ( rank == 0 ) {
        printf( "Total time (s): %f\n", maxTime );
    }

    /* shut down MPI */
    MPI_Finalize();
    return 0;
}