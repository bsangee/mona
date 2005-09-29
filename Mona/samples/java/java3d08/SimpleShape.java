
//------------------------------------------------------
//シンプルな図形を作成する。
//
//トーラス
//------------------------------------------------------
public class SimpleShape
{
	Vertex vertices[];
	int    indices [];

	//平面
	public void CreatePolygon(int n)
	{
		vertices = new Vertex[(n+1) * (n+1)];
		
		int p=0;
		float pz = -1.0f;
		float px = 0;
		float add = 2.0f / (float)n;
		for (int z=0;z<=n;z++,pz+=add){
			px = -1.0f;
			for (int x=0;x<=n;x++,px+=add){
				vertices[p++] = new Vertex(px,0,pz,0,1,0);
			}
		}
		
		indices = new int[n * n * 6];
		
		p = 0;
		for (int z=0;z<n;z++){
			for (int x=0;x<n;x++){
				indices[p++] = z * (n+1) + x;
				indices[p++] = z * (n+1) + x + 1;
				indices[p++] = (z+1) * (n+1) + x + 1;
				indices[p++] = (z+1) * (n+1) + x + 1;
				indices[p++] = (z+1) * (n+1) + x;
				indices[p++] = z * (n+1) + x;
			}
		}
		
		for (int i=0;i<vertices.length;i++){
			//UV座標設定
			vertices[i].tu = vertices[i].v.x/2 + .5f;
			vertices[i].tv = vertices[i].v.z/2 + .5f;
		}
	}
	
	//シリンダー（円錐）
	public void CreateCylinder(int n)
	{		
		int p = 0;
		float y1 = 1.0f;
		float y2 =-1.0f;
		float a  = 0;
		float add= (float)Math.PI/(n/2);
	
		//頂点データの作成
		vertices = new Vertex[n + 2];

		vertices[p++] = new Vertex(0,y1,0,0,0,0);
		vertices[p++] = new Vertex(0,y2,0,0,0,0);
		
		for (int i=0;i<n;i++){
			vertices[p++] = new Vertex((float)Math.cos(a)/4,y2,(float)Math.sin(a)/4,0,0,0);
			a+=add;
		}
		
		
		//インデックスデータの作成
		indices = new int[(n + n)*3];
	
		p = 0;
		for (int i=0;i<n;i++){
			indices[p++] = (i+1)%n + 2;
			indices[p++] = i + 2;
			indices[p++] = 0;

			indices[p++] = 1;
			indices[p++] = i + 2;
			indices[p++] = (i+1)%n + 2;
		}
		
		//グーローシェーディング用に法線ベクトルを平均化
		for (int i=0;i<indices.length;i+=3){
			int i1 = indices[i  ];
			int i2 = indices[i+1];
			int i3 = indices[i+2];
			//法線ベクトルを求める
			Vector vn = CalcNormal(vertices[i1].v,vertices[i2].v,vertices[i3].v);
			
			//頂点に法線ベクトルを加算
			vertices[i1].n.Add(vn);
			vertices[i2].n.Add(vn);
			vertices[i3].n.Add(vn);
			vertices[i1].used ++;
			vertices[i2].used ++;
			vertices[i3].used ++;
		}
		
		for (int i=0;i<vertices.length;i++){
			//平均化
			if (vertices[i].used > 0){
				vertices[i].n.x /= vertices[i].used;
				vertices[i].n.y /= vertices[i].used;
				vertices[i].n.z /= vertices[i].used;
			}
			//UV座標設定
			vertices[i].tu = vertices[i].v.x/2+.5f;
			vertices[i].tv = vertices[i].v.y/2+.5f;
		}
	}
	
	//トーラス（ドーナツ）
	public void CreateTorus(int n)
	{
		float a    = 0;
		float c    = 0;
		float addc = (float)Math.PI*2/n;
		int   p    = 0;
		
		//頂点データの作成
		vertices = new Vertex[n * n];
		
		for (int i=0;i<n;i++){
			float s = (float)Math.sin(c) + 2;
			float z = (float)Math.cos(c);
			for (int j=0;j<n;j++){
				float x = (float)(Math.cos(a));
				float y = (float)(Math.sin(a));
				vertices[p] = new Vertex();
				vertices[p].v.x = x * s * .4f;
				vertices[p].v.y = y * s * .4f;
				vertices[p].v.z = z * .4f;
				a+= Math.PI/n*2;
				p++;
			}
			c += addc;
		}
		
		//インデックスデータの作成
		indices = new int[n * n * 2 * 3];
		
		p = 0;
		int m = n*n;
		for (int j=0;j<n;j++){
			int off = j * n;
			for (int i=0;i<n;i++){
				indices[p++] = (off+n)%m+(i+1)%n;
				indices[p++] = off+(i+1)%n;
				indices[p++] = off+i;

				indices[p++] = (off+n)%m+i;
				indices[p++] = (off+n)%m+(i+1)%n;
				indices[p++] = off+i;
			}
		}
		
		//グーローシェーディング用に法線ベクトルを平均化
		for (int i=0;i<indices.length;i+=3){
			int i1 = indices[i  ];
			int i2 = indices[i+1];
			int i3 = indices[i+2];
			//法線ベクトルを求める
			Vector vn = CalcNormal(vertices[i1].v,vertices[i2].v,vertices[i3].v);
			
			//頂点に法線ベクトルを加算
			vertices[i1].n.Add(vn);
			vertices[i2].n.Add(vn);
			vertices[i3].n.Add(vn);
			vertices[i1].used ++;
			vertices[i2].used ++;
			vertices[i3].used ++;
		}
		
		for (int i=0;i<vertices.length;i++){
			//平均化
			if (vertices[i].used > 0){
				vertices[i].n.x /= vertices[i].used;
				vertices[i].n.y /= vertices[i].used;
				vertices[i].n.z /= vertices[i].used;
			}
			//UV座標設定
			vertices[i].tu = vertices[i].v.x/2+.5f;
			vertices[i].tv = vertices[i].v.y/2+.5f;
		}
	}
		
	//頂点バッファを返す
	public Vertex[] GetVB()
	{
		return vertices;
	}
	
	//インデックス配列を返す
	public int[] GetIndices()
	{
		return indices;
	}
	
	//--------------------------------------------------------------------
	//面に垂直な法線ベクトルを求める。
	//
	//--------------------------------------------------------------------
	static final private Vector CalcNormal(Vector v1,Vector v2,Vector v3)
	{
		Vector n = new Vector();
		Vector a = new Vector();
		Vector b = new Vector();
		
		//３頂点からa,bベクトルを求める。
		a.x = v1.x - v2.x;
		a.y = v1.y - v2.y;
		a.z = v1.z - v2.z;
		b.x = v1.x - v3.x;
		b.y = v1.y - v3.y;
		b.z = v1.z - v3.z;
				
		//外積を取る。
		n.CrossProduct(a,b);
		
		//法線ベクトルを正規化
		n.Normalize();
		n.Invert();
		
		return n;
	}
}